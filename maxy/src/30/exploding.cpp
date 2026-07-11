#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Exploding models";
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
    Shader exploding;
};

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string> create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &)            = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept       = delete;
    SceneRenderer &operator=(SceneRenderer &&o)     = delete;
    ~SceneRenderer() noexcept                       = default;

    void render(input_t input, float delta);

private:
    shaders_t m_shaders;
    Model     m_model;

    SceneRenderer(shaders_t shaders, Model model)
        : m_shaders{std::move(shaders)}, m_model(std::move(model)) {}
};

std::expected<shaders_t, std::string> load_shaders() {
    auto shader = Shader::build(
        "shaders/30_exploding.vert", "shaders/30_exploding.frag", "shaders/30_exploding.geom"
    );
    if (!shader) return std::unexpected(shader.error());
    return shaders_t{.exploding = std::move(*shader)};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    auto model = Model::load("objects/nanosuit/nanosuit.obj");
    if (!model) return std::unexpected(model.error());

    auto renderer = new SceneRenderer{std::move(*shaders), std::move(*model)};

    return std::unique_ptr<SceneRenderer>{renderer};
}

void SceneRenderer::render(input_t input, float delta) {
    process_camera_events(state.window, input, delta);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaders.exploding.use();

    glm::mat4 view       = state.window.camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(state.window.camera.fov),
        static_cast<float>(state.window.viewport.width) /
            static_cast<float>(state.window.viewport.height),
        .1f, 100.f
    );
    glm::mat4 model_transform = glm::mat4(1.f);

    m_shaders.exploding.set_mat4("view", view);
    m_shaders.exploding.set_mat4("projection", projection);
    m_shaders.exploding.set_mat4("model", model_transform);
    m_shaders.exploding.set_float("time", static_cast<float>(glfwGetTime()));

    m_model.draw(m_shaders.exploding);
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
