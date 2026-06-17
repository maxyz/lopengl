#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Using uniform buffer objects";
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
    Shader red;
    Shader green;
    Shader blue;
    Shader yellow;
};

struct vaos_t {
    id_t cube;
};
struct vbos_t {
    id_t cube;
    id_t matrices;
};
struct matrices_t {
    glm::mat4 projection;
    glm::mat4 view;
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
        : m_shaders{std::move(shaders)}, m_vaos{vaos}, m_vbos{vbos} {

        glm::mat4 projection = glm::perspective(
            glm::radians(state.window.camera.fov),
            static_cast<float>(state.window.viewport.width) /
                static_cast<float>(state.window.viewport.height),
            .1f, 100.f
        );
        buffers::uniform_block_memcpy(
            m_vbos.matrices, offsetof(matrices_t, projection), projection
        );
    };
};

std::expected<shaders_t, std::string> load_shaders() {
    auto red_shader = Shader::build("shaders/29_ubo.vert", "shaders/29_ubo_red.frag");
    if (!red_shader) return std::unexpected(red_shader.error());

    auto green_shader = Shader::build("shaders/29_ubo.vert", "shaders/29_ubo_green.frag");
    if (!green_shader) return std::unexpected(green_shader.error());

    auto blue_shader = Shader::build("shaders/29_ubo.vert", "shaders/29_ubo_blue.frag");
    if (!blue_shader) return std::unexpected(blue_shader.error());

    auto yellow_shader = Shader::build("shaders/29_ubo.vert", "shaders/29_ubo_yellow.frag");
    if (!yellow_shader) return std::unexpected(yellow_shader.error());

    return shaders_t{
        .red    = std::move(*red_shader),
        .green  = std::move(*green_shader),
        .blue   = std::move(*blue_shader),
        .yellow = std::move(*yellow_shader),
    };
}

std::pair<vaos_t, vbos_t> load_buffers(id_t uniform_buffer_index) {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(1, reinterpret_cast<id_t *>(&vaos));
    glGenBuffers(2, reinterpret_cast<id_t *>(&vbos));

    buffers::load_vertices(cube_vertices, vaos.cube, vbos.cube);
    buffers::uniform_block_alloc(vbos.matrices, uniform_buffer_index, 2 * sizeof(glm::mat4));

    return {vaos, vbos};
}

void link_shaders_uniform_buffer(shaders_t &shaders, id_t index) {
    id_t uniform_block_index_red   = glGetUniformBlockIndex(shaders.red.program_id(), "matrices");
    id_t uniform_block_index_green = glGetUniformBlockIndex(shaders.green.program_id(), "matrices");
    id_t uniform_block_index_blue  = glGetUniformBlockIndex(shaders.blue.program_id(), "matrices");
    id_t uniform_block_index_yellow =
        glGetUniformBlockIndex(shaders.yellow.program_id(), "matrices");

    glUniformBlockBinding(shaders.red.program_id(), uniform_block_index_red, index);
    glUniformBlockBinding(shaders.green.program_id(), uniform_block_index_green, index);
    glUniformBlockBinding(shaders.blue.program_id(), uniform_block_index_blue, index);
    glUniformBlockBinding(shaders.yellow.program_id(), uniform_block_index_yellow, index);
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    id_t uniform_buffer_index = 1;
    auto [vaos, vbos]         = load_buffers(uniform_buffer_index);

    link_shaders_uniform_buffer(*shaders, uniform_buffer_index);

    auto renderer = new SceneRenderer{std::move(*shaders), vaos, vbos};

    return std::unique_ptr<SceneRenderer>{renderer};
}

void SceneRenderer::render(input_t input, float delta) {
    process_camera_events(state.window, input, delta);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = state.window.camera.get_view_matrix();
    buffers::uniform_block_memcpy(m_vbos.matrices, offsetof(matrices_t, view), view);

    glBindVertexArray(m_vaos.cube);

    glm::mat4 model;

    m_shaders.red.use();
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, 0.75f, 0.0f));
    m_shaders.red.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    m_shaders.green.use();
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.0f));
    m_shaders.green.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    m_shaders.blue.use();
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, -0.75f, 0.0f));
    m_shaders.blue.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    m_shaders.yellow.use();
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, -0.75f, 0.0f));
    m_shaders.yellow.set_mat4("model", model);
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
