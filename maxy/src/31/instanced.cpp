#include <iostream>

#include "common/common.hpp"

constexpr const char *TITLE  = "Instanced Asteroids";
constexpr GLuint      WIDTH  = 1024;
constexpr GLuint      HEIGHT = 768;

struct state_t {
    window_state_t window;
};

state_t state{
    .window = {
        .viewport = {.width = WIDTH, .height = HEIGHT},
        .camera   = Camera{glm::vec3(0.f, 4.f, 155.f)},
    }
};

struct shaders_t {
    Shader planet;
    Shader asteroids;
};
struct models_t {
    Model rock;
    Model planet;
};
struct vbos_t {
    id_t instance;
};

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string> create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &)            = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept       = delete;
    SceneRenderer &operator=(SceneRenderer &&o)     = delete;
    ~SceneRenderer() noexcept {
        glDeleteBuffers(sizeof(m_vbos) / sizeof(id_t), reinterpret_cast<id_t *>(&m_vbos));
    }

    void render(input_t input, float delta);

private:
    static constexpr int     N = 100000;
    shaders_t                m_shaders;
    models_t                 m_models;
    vbos_t                   m_vbos;
    std::array<glm::mat4, N> m_model_transformations;

    SceneRenderer(shaders_t shaders, models_t models)
        : m_shaders{std::move(shaders)}, m_models(std::move(models)) {

        init_model_transformations();
        load_buffers();
    }

    void init_model_transformations() {
        float radius = 150.0;
        float offset = 2.5f;
        for (unsigned int i = 0; i < N; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle        = static_cast<float>(i) / static_cast<float>(N) * 360.0f;
            float displacement = random_float(0, 2 * offset) - offset;
            float x            = sin(angle) * radius + displacement;
            displacement       = random_float(0, 2 * offset) - offset;
            float y = displacement *
                      0.4f; // keep height of asteroid field smaller compared to width of x and z
            displacement = random_float(0, 2 * offset) - offset;
            float z      = cos(angle) * radius + displacement;
            model        = glm::translate(model, glm::vec3(x, y, z));

            // 2. scale: Scale between 0.05 and 0.25f
            float scale = random_float(0.05, 0.25);
            model       = glm::scale(model, glm::vec3(scale));

            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            float rot_angle = random_float(0, 360);
            model           = glm::rotate(model, rot_angle, glm::vec3(0.4f, 0.6f, 0.8f));

            // 4. now add to list of matrices
            m_model_transformations[i] = model;
        }
    }

    void load_buffers() {
        glGenBuffers(1, &m_vbos.instance);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbos.instance);
        glBufferData(
            GL_ARRAY_BUFFER, N * sizeof(glm::mat4), m_model_transformations.data(), GL_STATIC_DRAW
        );
        m_models.rock.set_instance_model_transform(3);
    }
};

std::expected<shaders_t, std::string> load_shaders() {
    auto planet_shader = Shader::build("shaders/31_planet.vert", "shaders/31_planet.frag");
    if (!planet_shader) return std::unexpected(planet_shader.error());

    auto asteroids_shader = Shader::build("shaders/31_asteroids.vert", "shaders/31_asteroids.frag");
    if (!asteroids_shader) return std::unexpected(asteroids_shader.error());

    return shaders_t{
        .planet = std::move(*planet_shader), .asteroids = std::move(*asteroids_shader)
    };
}

std::expected<models_t, std::string> load_models() {
    auto rock_model = Model::load("objects/rock/rock.obj");
    if (!rock_model) return std::unexpected(rock_model.error());

    auto planet_model = Model::load("objects/planet/planet.obj");
    if (!planet_model) return std::unexpected(planet_model.error());

    return models_t{std::move(*rock_model), std::move(*planet_model)};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto shaders = load_shaders();
    if (!shaders) return std::unexpected(shaders.error());

    auto models = load_models();
    if (!models) return std::unexpected(models.error());

    auto renderer = new SceneRenderer{std::move(*shaders), std::move(*models)};

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
    model_transform           = glm::translate(model_transform, glm::vec3(0.0f, -3.0f, 0.0f));
    model_transform           = glm::scale(model_transform, glm::vec3(4.0f, 4.0f, 4.0f));

    m_shaders.planet.use();
    m_shaders.planet.set_mat4("view", view);
    m_shaders.planet.set_mat4("projection", projection);
    m_shaders.planet.set_mat4("model", model_transform);

    m_models.planet.draw(m_shaders.planet);

    m_shaders.asteroids.use();
    m_shaders.asteroids.set_mat4("view", view);
    m_shaders.asteroids.set_mat4("projection", projection);
    m_models.rock.draw_instanced(m_shaders.asteroids, N);
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
