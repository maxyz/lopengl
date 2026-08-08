#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Gamma correction";
constexpr GLuint      WIDTH  = 1024;
constexpr GLuint      HEIGHT = 768;

inline const std::array<vertex_t, 6> large_floor_vertices = {{
    {{500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {200.f, 0.f}},
    {{-500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
    {{-500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {0.f, 200.f}},
    {{500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {200.f, 0.f}},
    {{-500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {0.f, 200.f}},
    {{500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {200.f, 200.f}},
}};


struct state_t {
    window_state_t window;
    std::vector<light_positional_t> pos_lights;
    bool gamma;
};

state_t state{
    .window = {
        .viewport = {.width = WIDTH, .height = HEIGHT},
        .camera   = Camera{glm::vec3(0.f, 1.f, 3.f)},
    },
    .pos_lights = {
        {.position = glm::vec3(-3.f, 1.f,  0.f),
             .ambient = glm::vec3(.0f,.0f,.0f), .diffuse = glm::vec3(.25f,.25f,.25f),
             .specular = glm::vec3(.25f,.25f,.25f), .constant = 1.f, .linear = 1.f, .quadratic = 1.f},
        {.position = glm::vec3(-1.f, 1.f,  0.f),
             .ambient = glm::vec3(.0f,.0f,.0f), .diffuse = glm::vec3(.5f,.5f,.5f),
             .specular = glm::vec3(.5f,.5f,.5f), .constant = 1.f, .linear = 1.f, .quadratic = 1.f},
        {.position = glm::vec3(1.f, 1.f,  0.f),
             .ambient = glm::vec3(.0f,.0f,.0f), .diffuse = glm::vec3(.75f,.75f,.75f),
             .specular = glm::vec3(.75f,.75f,.75f), .constant = 1.f, .linear = 1.f, .quadratic = 1.f},
        {.position = glm::vec3(3.f, 1.f,  0.f),
             .ambient = glm::vec3(.0f,.0f,.0f), .diffuse = glm::vec3(1.f,1.f,1.f),
             .specular = glm::vec3(1.f,1.f,1.f), .constant = 1.f, .linear = 1.f, .quadratic = 1.f},
    },
    .gamma = true,
};

struct shaders_t {
    Shader gamma;
};

struct vaos_t {
    id_t plane;
};
struct vbos_t {
    id_t plane;
};
struct textures_t {
    id_t wood;
    id_t wood_gamma;
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
    textures_t m_textures;
    vaos_t    m_vaos;
    vbos_t    m_vbos;

    SceneRenderer(shaders_t shaders, textures_t textures, vaos_t vaos, vbos_t vbos)
        : m_shaders{std::move(shaders)}, m_textures{textures}, m_vaos{vaos}, m_vbos{vbos} {}
};

std::expected<shaders_t, std::string> load_shaders() {
    auto shader = Shader::build("shaders/34_gamma.vert", "shaders/34_gamma.frag");
    if (!shader) return std::unexpected(shader.error());
    return shaders_t{.gamma = std::move(*shader)};
}

std::expected<textures_t, std::string> load_textures() {
    auto wood_texture = load_texture("textures/wood.png");
    if (!wood_texture) return std::unexpected(wood_texture.error());
    auto options = DEFAULT_TEXTURE_OPTIONS;
    options.gamma_correction = true;

    auto wood_gamma_texture = load_texture("textures/wood.png", options);
    if (!wood_gamma_texture) return std::unexpected(wood_gamma_texture.error());

    return textures_t{
        .wood = *wood_texture,
        .wood_gamma = *wood_gamma_texture,
    };
}


std::pair<vaos_t, vbos_t> load_buffers() {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(sizeof(vaos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vaos));
    glGenBuffers(sizeof(vbos_t) / sizeof(id_t), reinterpret_cast<id_t *>(&vbos));

    buffers::load_vertices(large_floor_vertices, vaos.plane, vbos.plane);

    return {vaos, vbos};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    auto textures = load_textures();
    if (!textures) return std::unexpected(textures.error());

    auto [vaos, vbos] = load_buffers();

    shaders->gamma.use();
    shaders->gamma.set_int("floor_texture", 0);

    auto renderer = new SceneRenderer{std::move(*shaders), *textures, vaos, vbos};

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

    m_shaders.gamma.use();
    m_shaders.gamma.set_mat4("view", view);
    m_shaders.gamma.set_mat4("projection", projection);
    // m_shaders.blinn.set_vec3("light_pos", state.pos_light.position);
    auto program = m_shaders.gamma.program_id();
    for (size_t i = 0; i < state.pos_lights.size(); ++i) {
        set_positional_light(program, std::format("positional_lights[{}]", i), state.pos_lights[i]);
    }
    m_shaders.gamma.set_vec3("view_pos", state.window.camera.position);
    m_shaders.gamma.set_int("gamma", state.gamma);

    glBindVertexArray(m_vaos.plane);
    glActiveTexture(GL_TEXTURE0);
    if (state.gamma) {
        glBindTexture(GL_TEXTURE_2D, m_textures.wood_gamma);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_textures.wood);
    }
    glDrawArrays(GL_TRIANGLES, 0, large_floor_vertices.size());
}

void key_callback_combined(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }
    key_callback(window, key, scancode, action, mods);

    if ((key == GLFW_KEY_G) && (action == GLFW_PRESS)) {
        if (mods & GLFW_MOD_SHIFT) {
            state.gamma = false;
        } else {
            state.gamma = true;
        }
    }
}

int error_exit(std::string error) {
    std::cerr << error << "\n";
    return -1;
}

int main() {
    auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
    if (!ctx) return error_exit(ctx.error());

    window_callbacks_t window_callbacks{DEFAULT_WINDOW_CALLBACKS};
    window_callbacks.key = key_callback_combined;
    init_window_callbacks(ctx->window(), state.window, window_callbacks);

    auto expected_renderer = SceneRenderer::create(ctx->window());
    if (!expected_renderer) return error_exit(expected_renderer.error());
    auto renderer = std::move(*expected_renderer);

    event_loop(ctx->window(), *renderer, process_common_input);
    return 0;
}
