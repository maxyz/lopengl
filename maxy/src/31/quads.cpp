#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Quad instances";
constexpr GLuint      WIDTH  = 1024;
constexpr GLuint      HEIGHT = 768;

struct state_t {
    window_state_t window;
};

state_t state{
    .window = {
        .viewport = {.width = WIDTH, .height = HEIGHT},
        .camera   = Camera{glm::vec3(0.f, 0.f, 3.f)},
    }
};

struct shaders_t {
    Shader quads;
};

struct vaos_t {
    id_t quad;
};
struct vbos_t {
    id_t quad;
    id_t instance;
};

struct position_color_t {
    glm::vec3 position;
    glm::vec4 color;
};

inline const std::array<position_color_t, 16> quad_vertices = {
    {{{-0.05f, 0.05f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
     {{0.05f, -0.05f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
     {{-0.05f, -0.05f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},

     {{-0.05f, 0.05f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
     {{0.05f, -0.05f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
     {{0.05f, 0.05f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}}}
};

constexpr std::array<glm::vec3, 100> generate_translations() {
    std::array<glm::vec3, 100> result{};
    int                        index  = 0;
    float                      offset = 0.1f;
    for (int y = -10; y < 10; y += 2) {
        for (int x = -10; x < 10; x += 2) {
            glm::vec3 translation{0.0f};
            translation.x   = (float)x / 10.0f + offset;
            translation.y   = (float)y / 10.0f + offset;
            result[index++] = translation;
        }
    }
    return result;
}
constexpr std::array<glm::vec3, 100> translations = generate_translations();

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string> create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &)            = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept       = delete;
    SceneRenderer &operator=(SceneRenderer &&o)     = delete;

    ~SceneRenderer() noexcept {
        glDeleteVertexArrays(sizeof(m_vaos) / sizeof(id_t), reinterpret_cast<id_t *>(&m_vaos));
        glDeleteBuffers(sizeof(m_vbos) / sizeof(id_t), reinterpret_cast<id_t *>(&m_vbos));
    }

    void render(input_t input, float delta);

private:
    shaders_t m_shaders;
    vaos_t    m_vaos;
    vbos_t    m_vbos;

    SceneRenderer(shaders_t shaders, vaos_t vaos, vbos_t vbos)
        : m_shaders{std::move(shaders)}, m_vaos{vaos}, m_vbos{vbos} {}
};

std::expected<shaders_t, std::string> load_shaders() {
    auto shader = Shader::build("shaders/31_instancing.vert", "shaders/31_instancing.frag");
    if (!shader) return std::unexpected(shader.error());
    return shaders_t{.quads = std::move(*shader)};
}

void load_vertices(
    std::span<const position_color_t> vertices, id_t vertex_array_object, id_t vertex_buffer_object
) {
    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(position_color_t),
        reinterpret_cast<void *>(offsetof(position_color_t, position))
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(position_color_t),
        reinterpret_cast<void *>(offsetof(position_color_t, color))
    );
    glEnableVertexAttribArray(1);
}

std::pair<vaos_t, vbos_t> load_buffers() {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(1, reinterpret_cast<id_t *>(&vaos));
    glGenBuffers(2, reinterpret_cast<id_t *>(&vbos));

    load_vertices(quad_vertices, vaos.quad, vbos.quad);

    glBindBuffer(GL_ARRAY_BUFFER, vbos.instance);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(glm::vec3) * translations.size(), translations.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

    return {vaos, vbos};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    auto [vaos, vbos] = load_buffers();

    auto renderer = new SceneRenderer{std::move(*shaders), vaos, vbos};

    return std::unique_ptr<SceneRenderer>{renderer};
}

void SceneRenderer::render(input_t input, float delta) {
    process_camera_events(state.window, input, delta);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaders.quads.use();
    glBindVertexArray(m_vaos.quad);
    glDrawArraysInstanced(GL_TRIANGLES, 0, quad_vertices.size(), 100);
}

int error_exit(std::string error) {
    std::cerr << error << "\n";
    return -1;
}

int main() {
    auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
    if (!ctx) return error_exit(ctx.error());

    init_window_callbacks(ctx->window(), state.window);

    auto expected_renderer = SceneRenderer::create(ctx->window());
    if (!expected_renderer) return error_exit(expected_renderer.error());
    auto renderer = std::move(*expected_renderer);

    event_loop(ctx->window(), *renderer, process_common_input);
    return 0;
}
