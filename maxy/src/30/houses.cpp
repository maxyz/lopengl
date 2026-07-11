#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Points as houses";
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
    Shader houses;
};

struct vaos_t {
    id_t points;
};
struct vbos_t {
    id_t points;
};

struct position_color_t {
    glm::vec3 position;
    glm::vec4 color;
};

inline const std::array<position_color_t, 4> points_vertices = {{
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // top-left
    {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},   // top-right
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}, // bottom-left
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},  // bottom-right
}};

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string> create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &)            = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept       = delete;
    SceneRenderer &operator=(SceneRenderer &&o)     = delete;

    ~SceneRenderer() noexcept {}

    void render(input_t input, float delta);

private:
    shaders_t m_shaders;
    vaos_t    m_vaos;
    vbos_t    m_vbos;

    SceneRenderer(shaders_t shaders, vaos_t vaos, vbos_t vbos)
        : m_shaders{std::move(shaders)}, m_vaos{vaos}, m_vbos{vbos} {}
};

std::expected<shaders_t, std::string> load_shaders() {
    auto shader =
        Shader::build("shaders/30_houses.vert", "shaders/30_houses.frag", "shaders/30_houses.geom");
    if (!shader) return std::unexpected(shader.error());
    return shaders_t{.houses = std::move(*shader)};
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
    glGenBuffers(1, reinterpret_cast<id_t *>(&vbos));

    load_vertices(points_vertices, vaos.points, vbos.points);

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

    m_shaders.houses.use();
    glBindVertexArray(m_vaos.points);
    glDrawArrays(GL_POINTS, 0, points_vertices.size());
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
