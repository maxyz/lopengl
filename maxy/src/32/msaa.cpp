#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Multi sampling anti aliasing";
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
    Shader msaa;
};

struct vaos_t {
    id_t cube;
};
struct vbos_t {
    id_t cube;
};

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
    auto shader = Shader::build("shaders/32_msaa.vert", "shaders/32_msaa.frag");
    if (!shader) return std::unexpected(shader.error());
    return shaders_t{.msaa = std::move(*shader)};
}

std::pair<vaos_t, vbos_t> load_buffers() {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(sizeof(vaos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vaos));
    glGenBuffers(sizeof(vbos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vbos));

    buffers::load_vertices(cube_vertices, vaos.cube, vbos.cube);

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

    glm::mat4 view       = state.window.camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(state.window.camera.fov),
        static_cast<float>(state.window.viewport.width) /
            static_cast<float>(state.window.viewport.height),
        .1f, 1000.f
    );
    glm::mat4 model_transform = glm::mat4(1.f);

    m_shaders.msaa.use();
    m_shaders.msaa.set_mat4("view", view);
    m_shaders.msaa.set_mat4("projection", projection);
    m_shaders.msaa.set_mat4("model", model_transform);

    glBindVertexArray(m_vaos.cube);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
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
