#include <expected>
#include <iostream>
#include <map>
#include <random>
#include <ranges>
#include <utility>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/common.hpp"

constexpr const char *TITLE = "Transparent windows";
constexpr GLuint WIDTH = 1024;
constexpr GLuint HEIGHT = 768;

struct model_preset_t {
    glm::vec3 position;
    float scale;
};
struct preset_t {
    std::string name;
    glm::vec4 clear_color;
    std::vector<model_preset_t> cubes;
    std::vector<model_preset_t> windows;
    model_preset_t plane;
    light_directional_t dir_light;
    std::vector<light_positional_t> pos_lights;
    std::vector<light_spot_t> spot_lights;
    flashlight_t flashlight;
};

struct depth_mode_t {
    std::string name;
    GLenum mode;
};

constexpr int MAX_POS_LIGHTS = 16;
constexpr int MAX_SPOT_LIGHTS = 8;

struct state_t {
    window_state_t window = {
        .viewport = {.width = WIDTH, .height = HEIGHT},
        .camera = Camera{glm::vec3(0.f, .5f, 3.f)},
    };
    size_t preset_index = 0;
    size_t depth_mode_index = 2; // Initial value set to less
    std::vector<light_positional_t> pos_lights{};
    std::vector<light_spot_t> spot_lights{};
    bool flashlight_on = true;
};
state_t state;

float random_float(float lo, float hi) {
    static std::mt19937 rng{std::random_device{}()};
    return std::uniform_real_distribution<float>{lo, hi}(rng);
}

light_positional_t random_positional_light() {
    glm::vec3 color{
        random_float(0.1f, 1.f), random_float(0.1f, 1.f),
        random_float(0.1f, 1.f)
    };
    return {
        .position =
            {random_float(-5.f, 5.f), random_float(-5.f, 5.f),
             random_float(-5.f, 5.f)},
        .ambient = color * 0.1f,
        .diffuse = color,
        .specular = color,
        .constant = 1.f,
        .linear = random_float(0.07f, 0.22f),
        .quadratic = random_float(0.017f, 0.07f),
    };
}

light_spot_t random_spot_light() {
    glm::vec3 color{
        random_float(0.1f, 1.f), random_float(0.1f, 1.f),
        random_float(0.1f, 1.f)
    };
    float cutoff_deg = random_float(10.f, 25.f);
    float outer_cutoff_deg = cutoff_deg + random_float(2.f, 8.f);
    glm::vec3 dir{
        random_float(-1.f, 1.f), random_float(-1.f, 1.f),
        random_float(-1.f, 1.f)
    };
    return {
        .position =
            {random_float(-5.f, 5.f), random_float(-5.f, 5.f),
             random_float(-5.f, 5.f)},
        .direction = glm::normalize(dir),
        .ambient = color * 0.05f,
        .diffuse = color,
        .specular = color,
        .cutoff = glm::cos(glm::radians(cutoff_deg)),
        .outer_cutoff = glm::cos(glm::radians(outer_cutoff_deg)),
        .constant = 1.f,
        .linear = random_float(0.07f, 0.22f),
        .quadratic = random_float(0.017f, 0.07f),
    };
}

struct shaders_t {
    Shader view;
    Shader light_marker;
};
struct vaos_t {
    id_t cube;
    id_t window;
    id_t plane;
    id_t light_cube;
    id_t pyramid;
};
struct vbos_t {
    id_t cube;
    id_t window;
    id_t plane;
    id_t pyramid;
    id_t pyramid_ebo;
};
struct textures_t {
    id_t marble;
    id_t metal;
    id_t window;
    id_t white;
};

class SceneRenderer {
public:
    static std::expected<std::unique_ptr<SceneRenderer>, std::string>
    create(GLFWwindow *window);

    SceneRenderer(const SceneRenderer &) = delete;
    SceneRenderer &operator=(const SceneRenderer &) = delete;
    SceneRenderer(SceneRenderer &&o) noexcept = delete;
    SceneRenderer &operator=(SceneRenderer &&o) = delete;
    ~SceneRenderer() noexcept {
        glDeleteVertexArrays(5, &m_vaos.cube);
        glDeleteBuffers(5, &m_vbos.cube);
    }

    void render(input_t input, float delta);

private:
    GLFWwindow *m_window{};
    shaders_t m_shaders;
    textures_t m_textures{};
    vaos_t m_vaos;
    vbos_t m_vbos;

    SceneRenderer(
        GLFWwindow *window, shaders_t shaders, textures_t textures, vaos_t vaos,
        vbos_t vbos
    )
        : m_window{window}, m_shaders{std::move(shaders)}, m_textures(textures),
          m_vaos{vaos}, m_vbos(vbos) {};

    void render_scene();
    void render_fill_pass();
    void render_scene_set_lighting();
    void render_scene_draw_lights();
    void render_scene_draw_cubes();
    void render_scene_draw_windows();
    void render_scene_draw_plane();
    void render_imgui();
};

const std::vector<model_preset_t> simple_cubes = {
    {.position = glm::vec3(-2.f, .5f, -1.5f), .scale = 1.f},
    {.position = glm::vec3(2.f, 1.f, -2.f), .scale = 2.f},
};
// Desert: 4 windows scattered wide - sparse outpost feel
const std::vector<model_preset_t> desert_windows = {
    {.position = glm::vec3(-3.5f, 0.f, 0.f), .scale = 1.f},
    {.position = glm::vec3(3.f, 0.f, 0.5f), .scale = 1.1f},
    {.position = glm::vec3(0.f, 0.f, -0.5f), .scale = 1.2f},
    {.position = glm::vec3(-1.5f, 0.f, 1.5f), .scale = .85f},
};
// Factory: 5 windows in a regular row - industrial spacing
const std::vector<model_preset_t> factory_windows = {
    {.position = glm::vec3(-2.f, 0.f, -0.5f), .scale = .8f},
    {.position = glm::vec3(-1.f, 0.f, -0.5f), .scale = .8f},
    {.position = glm::vec3(0.f, 0.f, -0.5f), .scale = .8f},
    {.position = glm::vec3(1.f, 0.f, -0.5f), .scale = .8f},
    {.position = glm::vec3(2.f, 0.f, -0.5f), .scale = .8f},
};
// Horror: 2 windows — one looming close, one barely visible far away
const std::vector<model_preset_t> horror_windows = {
    {.position = glm::vec3(.5f, 0.f, 1.f), .scale = 1.3f},
    {.position = glm::vec3(-.5f, 0.f, -4.f), .scale = 1.5f},
};
// Biochemical: 6 windows in a 2-column grid — lab viewport arrangement
const std::vector<model_preset_t> biochemical_windows = {
    {.position = glm::vec3(-.6f, 0.f, .5f), .scale = .7f},
    {.position = glm::vec3(.6f, 0.f, .5f), .scale = .7f},
    {.position = glm::vec3(-.6f, 0.f, -.3f), .scale = .7f},
    {.position = glm::vec3(.6f, 0.f, -.3f), .scale = .7f},
    {.position = glm::vec3(-.6f, 0.f, -1.1f), .scale = .7f},
    {.position = glm::vec3(.6f, 0.f, -1.1f), .scale = .7f},
};
const model_preset_t simple_plane = {
    .position = glm::vec3(0.f, 0.f, 0.f),
    .scale = 1.f,
};

// Far clip is 100 units; ±500 ensures the floor edge is never visible.
// Texture tiles at the same density as floor_vertices (5 units per tile).
inline const std::array<vertex_t, 6> large_floor_vertices = {{
    {{500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {200.f, 0.f}},
    {{-500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
    {{-500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {0.f, 200.f}},
    {{500.f, 0.f, 500.f}, {0.f, 1.f, 0.f}, {200.f, 0.f}},
    {{-500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {0.f, 200.f}},
    {{500.f, 0.f, -500.f}, {0.f, 1.f, 0.f}, {200.f, 200.f}},
}};

const std::array<preset_t, 4> presets = {{
    {
        .name = "desert",
        .clear_color = glm::vec4(.75f, .52f, .3f, 1.f),
        .cubes = simple_cubes,
        .windows = desert_windows,
        .plane = simple_plane,
        .dir_light = {
            .direction = glm::vec3(-.2f, -1.f, -.3f),
            .ambient  = glm::vec3(.3f, .24f, .14f),
            .diffuse  = glm::vec3(.7f, .42f, .26f),
            .specular = glm::vec3(.5f, .5f,  .5f),
        },
        // Warm amber torch + distant red ember
        .pos_lights = {
            {.position = glm::vec3(0.f, 1.5f, 0.f),
             .ambient = glm::vec3(.1f, .06f, 0.f), .diffuse = glm::vec3(1.f, .6f, 0.f),
             .specular = glm::vec3(1.f, .6f, 0.f), .constant = 1.f, .linear = .14f, .quadratic = .07f},
            {.position = glm::vec3(-3.f, 2.f, -2.f),
             .ambient = glm::vec3(.08f, .01f, 0.f), .diffuse = glm::vec3(.8f, .1f, 0.f),
             .specular = glm::vec3(.8f, .1f, 0.f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
        },
        // Overhead lantern angled forward
        .spot_lights = {
            {.position = glm::vec3(0.f, 2.f, 1.f), .direction = glm::vec3(0.f, -1.f, -.3f),
             .ambient = glm::vec3(0.f, 0.f, 0.f), .diffuse = glm::vec3(.9f, .7f, .4f),
             .specular = glm::vec3(1.f, .9f, .6f),
             .cutoff = glm::cos(glm::radians(20.f)), .outer_cutoff = glm::cos(glm::radians(27.f)),
             .constant = 1.f, .linear = .09f, .quadratic = .032f},
        },
        .flashlight = {
            .ambient = glm::vec3(0.f, 0.f, 0.f), .diffuse = glm::vec3(.5f, .5f, .5f),
            .specular = glm::vec3(1.f, 1.f, 1.f),
            .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
            .constant = 1.f, .linear = .09f, .quadratic = .032f,
        },
    },
    {
        .name = "factory",
        .clear_color = glm::vec4(.1f, .1f, .1f, 1.f),
        .cubes = simple_cubes,
        .windows = factory_windows,
        .plane = simple_plane,
        .dir_light = {
            .direction = glm::vec3(-.2f, -1.f, -.3f),
            .ambient  = glm::vec3(.05f, .05f, .1f),
            .diffuse  = glm::vec3(.2f,  .2f,  .7f),
            .specular = glm::vec3(.7f,  .7f,  .7f),
        },
        // Four blue industrial lamps at the corners
        .pos_lights = {
            {.position = glm::vec3(-2.5f, 2.5f,  0.5f),
             .ambient = glm::vec3(.02f,.02f,.06f), .diffuse = glm::vec3(.2f,.2f,.6f),
             .specular = glm::vec3(.2f,.2f,.6f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 2.5f, 2.5f,  0.5f),
             .ambient = glm::vec3(.02f,.02f,.06f), .diffuse = glm::vec3(.2f,.2f,.6f),
             .specular = glm::vec3(.2f,.2f,.6f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3(-2.5f, 2.5f, -1.5f),
             .ambient = glm::vec3(.02f,.02f,.06f), .diffuse = glm::vec3(.2f,.2f,.6f),
             .specular = glm::vec3(.2f,.2f,.6f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 2.5f, 2.5f, -1.5f),
             .ambient = glm::vec3(.02f,.02f,.06f), .diffuse = glm::vec3(.2f,.2f,.6f),
             .specular = glm::vec3(.2f,.2f,.6f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
        },
        // Three harsh white work lights aimed straight down
        .spot_lights = {
            {.position = glm::vec3(-2.f, 3.f, -.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(1.f,1.f,1.f),
             .specular = glm::vec3(1.f,1.f,1.f),
             .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
             .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 0.f, 3.f, -.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(1.f,1.f,1.f),
             .specular = glm::vec3(1.f,1.f,1.f),
             .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
             .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 2.f, 3.f, -.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(1.f,1.f,1.f),
             .specular = glm::vec3(1.f,1.f,1.f),
             .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
             .constant = 1.f, .linear = .09f, .quadratic = .032f},
        },
        .flashlight = {
            .ambient = glm::vec3(0.f, 0.f, 0.f), .diffuse = glm::vec3(1.f, 1.f, 1.f),
            .specular = glm::vec3(1.f, 1.f, 1.f),
            .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
            .constant = 1.f, .linear = .09f, .quadratic = .032f,
        },
    },
    {
        .name = "horror",
        .clear_color = glm::vec4(.0f, .0f, .0f, 1.f),
        .cubes = simple_cubes,
        .windows = horror_windows,
        .plane = simple_plane,
        .dir_light = {
            .direction = glm::vec3(-.2f, -1.f, -.3f),
            .ambient  = glm::vec3(0.f,   0.f,   0.f),
            .diffuse  = glm::vec3(.05f, .05f,  .05f),
            .specular = glm::vec3(.2f,  .2f,   .2f),
        },
        // Single lone red ember
        .pos_lights = {
            {.position = glm::vec3(0.f, 1.f, -3.f),
             .ambient = glm::vec3(.03f, 0.f, 0.f), .diffuse = glm::vec3(.3f, 0.f, 0.f),
             .specular = glm::vec3(.3f, 0.f, 0.f), .constant = 1.f, .linear = .14f, .quadratic = .07f},
        },
        // One narrow dim overhead cone
        .spot_lights = {
            {.position = glm::vec3(0.f, 2.5f, 0.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(.15f,.15f,.15f),
             .specular = glm::vec3(.2f,.2f,.2f),
             .cutoff = glm::cos(glm::radians(8.f)), .outer_cutoff = glm::cos(glm::radians(12.f)),
             .constant = 1.f, .linear = .14f, .quadratic = .07f},
        },
        .flashlight = {
            .ambient = glm::vec3(0.f, 0.f, 0.f), .diffuse = glm::vec3(.5f, .5f, .5f),
            .specular = glm::vec3(1.f, 1.f, 1.f),
            .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
            .constant = 1.f, .linear = .09f, .quadratic = .032f,
        },
    },
    {
        .name = "biochemical",
        .clear_color = glm::vec4(.9f, .9f, .9f, 1.f),
        .cubes = simple_cubes,
        .windows = biochemical_windows,
        .plane = simple_plane,
        .dir_light = {
            .direction = glm::vec3(-.2f, -1.f, -.3f),
            .ambient  = glm::vec3(.5f, .5f, .5f),
            .diffuse  = glm::vec3(1.f, 1.f, 1.f),
            .specular = glm::vec3(1.f, 1.f, 1.f),
        },
        // Four corner lamps + bright centre overhead
        .pos_lights = {
            {.position = glm::vec3(-3.f, 2.5f,  0.5f),
             .ambient = glm::vec3(.04f,.07f,.01f), .diffuse = glm::vec3(.4f,.7f,.1f),
             .specular = glm::vec3(.4f,.7f,.1f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 3.f, 2.5f,  0.5f),
             .ambient = glm::vec3(.04f,.07f,.01f), .diffuse = glm::vec3(.4f,.7f,.1f),
             .specular = glm::vec3(.4f,.7f,.1f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3(-3.f, 2.5f, -1.5f),
             .ambient = glm::vec3(.04f,.07f,.01f), .diffuse = glm::vec3(.4f,.7f,.1f),
             .specular = glm::vec3(.4f,.7f,.1f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 3.f, 2.5f, -1.5f),
             .ambient = glm::vec3(.04f,.07f,.01f), .diffuse = glm::vec3(.4f,.7f,.1f),
             .specular = glm::vec3(.4f,.7f,.1f), .constant = 1.f, .linear = .09f, .quadratic = .032f},
            {.position = glm::vec3( 0.f, 3.f,  -0.5f),
             .ambient = glm::vec3(.06f,.1f,.02f), .diffuse = glm::vec3(.6f,1.f,.2f),
             .specular = glm::vec3(.6f,1.f,.2f), .constant = 1.f, .linear = .07f, .quadratic = .017f},
        },
        // Two focused green beams aimed down
        .spot_lights = {
            {.position = glm::vec3( 1.f, 2.5f, -.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(0.f,1.f,0.f),
             .specular = glm::vec3(0.f,1.f,0.f),
             .cutoff = glm::cos(glm::radians(25.f)), .outer_cutoff = glm::cos(glm::radians(30.f)),
             .constant = 1.f, .linear = .07f, .quadratic = .017f},
            {.position = glm::vec3(-1.f, 2.5f, -.5f), .direction = glm::vec3(0.f, -1.f, 0.f),
             .ambient = glm::vec3(0.f,0.f,0.f), .diffuse = glm::vec3(0.f,1.f,0.f),
             .specular = glm::vec3(0.f,1.f,0.f),
             .cutoff = glm::cos(glm::radians(25.f)), .outer_cutoff = glm::cos(glm::radians(30.f)),
             .constant = 1.f, .linear = .07f, .quadratic = .017f},
        },
        .flashlight = {
            .ambient = glm::vec3(0.f, 0.f, 0.f), .diffuse = glm::vec3(0.f, 1.f, 0.f),
            .specular = glm::vec3(0.f, 1.f, 0.f),
            .cutoff = glm::cos(glm::radians(15.f)), .outer_cutoff = glm::cos(glm::radians(20.f)),
            .constant = 1.f, .linear = .07f, .quadratic = .017f,
        },
    },
}};

void init_state() {
    const auto &preset = presets[state.preset_index];
    state.pos_lights.assign(preset.pos_lights.begin(), preset.pos_lights.end());
    state.spot_lights.assign(
        preset.spot_lights.begin(), preset.spot_lights.end()
    );
}

const std::array<depth_mode_t, 8> depth_modes = {{
    {
        .name = "always",
        .mode = GL_ALWAYS,
    },
    {
        .name = "never",
        .mode = GL_NEVER,
    },
    {
        .name = "less",
        .mode = GL_LESS,
    },
    {
        .name = "equal",
        .mode = GL_EQUAL,
    },
    {
        .name = "less equal",
        .mode = GL_LEQUAL,
    },
    {
        .name = "greater",
        .mode = GL_GREATER,
    },
    {
        .name = "not equal",
        .mode = GL_NOTEQUAL,
    },
    {
        .name = "greater equal",
        .mode = GL_GEQUAL,
    },
}};

id_t generate_white_texture() {
    id_t texture{};
    constexpr uint8_t white_pixel[] = {255, 255, 255};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white_pixel
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
}

std::expected<textures_t, std::string> load_textures() {
    auto marble_texture = load_texture("textures/marble.jpg");
    if (!marble_texture) {
        return std::unexpected(marble_texture.error());
    }
    auto metal_texture = load_texture("textures/metal.png");
    if (!metal_texture) {
        return std::unexpected(metal_texture.error());
    }
    texture_options_t window_options{DEFAULT_TEXTURE_OPTIONS};
    window_options.wrap = GL_CLAMP_TO_EDGE;
    auto window_texture = load_texture("textures/window.png", window_options);
    if (!window_texture) {
        return std::unexpected(window_texture.error());
    }
    return textures_t{
        .marble = *marble_texture,
        .metal = *metal_texture,
        .window = *window_texture,
        .white = generate_white_texture(),
    };
}

void load_buffer_vertices(
    std::span<const vertex_t> vertices, id_t vertex_array_object,
    id_t vertex_buffer_object
) {

    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(
        GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, position))
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, normal))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, texcoord))
    );
    glEnableVertexAttribArray(2);
}

std::pair<vaos_t, vbos_t> load_buffers() {
    vaos_t vaos{};
    vbos_t vbos{};

    glGenVertexArrays(5, &vaos.cube);
    glGenBuffers(5, &vbos.cube);

    load_buffer_vertices(cube_vertices, vaos.cube, vbos.cube);
    load_buffer_vertices(square_vertices, vaos.window, vbos.window);
    load_buffer_vertices(large_floor_vertices, vaos.plane, vbos.plane);

    // Light cube: shares the cube VBO, position only (attrib 0)
    glBindVertexArray(vaos.light_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbos.cube);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, position))
    );
    glEnableVertexAttribArray(0);

    // Pyramid: own VBO + EBO, packed vec3 positions (no normal/texcoord)
    glBindVertexArray(vaos.pyramid);
    glBindBuffer(GL_ARRAY_BUFFER, vbos.pyramid);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(pyramid_vertices), pyramid_vertices,
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.pyramid_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramid_indices), pyramid_indices,
        GL_STATIC_DRAW
    );
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void *>(0)
    );
    glEnableVertexAttribArray(0);

    return {vaos, vbos};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
    auto view_shader =
        Shader::build("shaders/25_lit.vert", "shaders/25_lit.frag");
    if (!view_shader)
        return std::unexpected(view_shader.error());

    auto light_marker_shader = Shader::build(
        "shaders/25_light_marker.vert", "shaders/25_light_marker.frag"
    );
    if (!light_marker_shader)
        return std::unexpected(light_marker_shader.error());

    auto textures = load_textures();
    if (!textures)
        return std::unexpected(textures.error());

    auto [vaos, vbos] = load_buffers();

    shaders_t shaders{
        .view = std::move(*view_shader),
        .light_marker = std::move(*light_marker_shader),
    };
    shaders.view.use();
    shaders.view.set_int("material.diffuse", 0);
    shaders.view.set_int("material.specular", 1);
    shaders.view.set_float("normal_flip", 1.0f);

    return std::unique_ptr<SceneRenderer>{
        new SceneRenderer{window, std::move(shaders), *textures, vaos, vbos}
    };
}

void SceneRenderer::render(input_t input, float delta) {
    process_camera_events(state.window, input, delta);

    const preset_t &preset = presets[state.preset_index];
    glClearColor(
        preset.clear_color.x, preset.clear_color.y, preset.clear_color.z,
        preset.clear_color.w
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    render_scene();

    render_imgui();
}

void SceneRenderer::render_scene() { render_fill_pass(); }

void SceneRenderer::render_fill_pass() {
    render_scene_set_lighting();
    render_scene_draw_plane();
    render_scene_draw_cubes();
    render_scene_draw_lights();
    render_scene_draw_windows();
}

void SceneRenderer::render_scene_set_lighting() {
    const preset_t &preset = presets[state.preset_index];

    glm::mat4 view = state.window.camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(state.window.camera.fov),
        static_cast<float>(state.window.viewport.width) /
            static_cast<float>(state.window.viewport.height),
        .1f, 100.f
    );

    m_shaders.view.use();
    m_shaders.view.set_mat4("view", view);
    m_shaders.view.set_mat4("projection", projection);
    m_shaders.view.set_vec3("view_pos", state.window.camera.position);
    m_shaders.view.set_float("material.shininess", 64.f);

    id_t program = m_shaders.view.program_id();
    set_directional_light(program, "dir_light", preset.dir_light);

    m_shaders.view.set_int(
        "pos_light_count", static_cast<int>(state.pos_lights.size())
    );
    for (size_t index = 0; index < state.pos_lights.size(); ++index)
        set_positional_light(
            program, std::format("pos_lights[{}]", index),
            state.pos_lights[index]
        );

    m_shaders.view.set_int(
        "spot_light_count", static_cast<int>(state.spot_lights.size())
    );
    for (size_t index = 0; index < state.spot_lights.size(); ++index)
        set_spot_light(
            program, std::format("spot_lights[{}]", index),
            state.spot_lights[index]
        );

    const flashlight_t active_flashlight =
        state.flashlight_on
            ? preset.flashlight
            : flashlight_t{.cutoff = 1.f, .outer_cutoff = 0.f, .constant = 1.f};
    set_flashlight(program, "flashlight", active_flashlight);
}

void SceneRenderer::render_scene_draw_lights() {
    glm::mat4 view = state.window.camera.get_view_matrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(state.window.camera.fov),
        static_cast<float>(state.window.viewport.width) /
            static_cast<float>(state.window.viewport.height),
        .1f, 100.f
    );

    m_shaders.light_marker.use();
    m_shaders.light_marker.set_mat4("view", view);
    m_shaders.light_marker.set_mat4("projection", projection);
    id_t program = m_shaders.light_marker.program_id();

    glBindVertexArray(m_vaos.light_cube);
    for (const light_positional_t &pos_light : state.pos_lights) {
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.f), pos_light.position), glm::vec3(.2f)
        );
        m_shaders.light_marker.set_mat4("model", model);
        set_positional_light(program, "light", pos_light);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    }

    glBindVertexArray(m_vaos.pyramid);
    for (const light_spot_t &spot_light : state.spot_lights) {
        glm::mat4 look_at = glm::lookAt(
            glm::vec3(0.f), spot_light.direction, glm::vec3(0, 1, 0)
        );
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.f), spot_light.position) *
                glm::inverse(look_at),
            glm::vec3(.2f)
        );
        m_shaders.light_marker.set_mat4("model", model);
        set_spot_light(program, "light", spot_light);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    }
}

void SceneRenderer::render_scene_draw_cubes() {
    const preset_t &preset = presets[state.preset_index];

    glBindVertexArray(m_vaos.cube);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures.metal);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures.metal);

    m_shaders.view.set_float("material.shininess", 32.f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for (const auto &model_info : preset.cubes) {
        glm::mat4 model_transform = glm::mat4(1.f);
        model_transform = glm::translate(model_transform, model_info.position);
        model_transform =
            glm::scale(model_transform, glm::vec3(model_info.scale));
        m_shaders.view.set_mat4("model", model_transform);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
    }

    glDisable(GL_CULL_FACE);
}

void SceneRenderer::render_scene_draw_windows() {
    const preset_t &preset = presets[state.preset_index];

    std::multimap<float, model_preset_t> sorted{};
    for (const auto &window_model : preset.windows) {
        float distance =
            glm::length(state.window.camera.position - window_model.position);
        sorted.insert({distance, window_model});
    }

    m_shaders.view.use();
    glBindVertexArray(m_vaos.window);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures.window);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures.white);

    glDepthMask(GL_FALSE);

    m_shaders.view.set_float("material.shininess", 128.f);

    glEnable(GL_CULL_FACE);

    glCullFace(GL_FRONT);
    m_shaders.view.set_float("normal_flip", -1.f);
    for (const auto &[_, model_info] : std::views::reverse(sorted)) {
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.f), model_info.position),
            glm::vec3(model_info.scale)
        );
        m_shaders.view.set_mat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, square_vertices.size());
    }

    glCullFace(GL_BACK);
    m_shaders.view.set_float("normal_flip", 1.f);
    for (const auto &[_, model_info] : std::views::reverse(sorted)) {
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.f), model_info.position),
            glm::vec3(model_info.scale)
        );
        m_shaders.view.set_mat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, square_vertices.size());
    }

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

void SceneRenderer::render_scene_draw_plane() {
    const preset_t &preset = presets[state.preset_index];

    glBindVertexArray(m_vaos.plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures.marble);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures.marble);

    m_shaders.view.set_float("material.shininess", 32.f);

    const auto &model_info = preset.plane;
    glm::mat4 model_transform = glm::mat4(1.f);
    model_transform = glm::translate(model_transform, model_info.position);
    model_transform = glm::scale(model_transform, glm::vec3(model_info.scale));

    m_shaders.view.set_mat4("model", model_transform);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.f, -1.f);
    glDrawArrays(GL_TRIANGLES, 0, large_floor_vertices.size());
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneRenderer::render_imgui() {
    ImGuiIO &io = ImGui::GetIO();
    auto cursor_input_mode = glfwGetInputMode(m_window, GLFW_CURSOR);
    bool camera_mode = cursor_input_mode != GLFW_CURSOR_NORMAL;

    if (camera_mode) {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    } else {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin(
        "Scene information", nullptr, ImGuiWindowFlags_AlwaysAutoResize
    );
    ImGui::PushItemWidth(150.0f);

    ImGui::LabelText(
        "Pos", "(%.2f, %.2f, %.2f)", state.window.camera.position.x,
        state.window.camera.position.y, state.window.camera.position.z
    );
    ImGui::LabelText("Preset", "%s", presets[state.preset_index].name.c_str());
    ImGui::LabelText(
        "Depth Mode", "%s", depth_modes[state.depth_mode_index].name.c_str()
    );
    ImGui::LabelText(
        "Cursor mode", "%s",
        cursor_input_mode == GLFW_CURSOR_DISABLED ? "DISABLED"
        : cursor_input_mode == GLFW_CURSOR_HIDDEN ? "HIDDEN"
                                                  : "NORMAL"
    );

    if (camera_mode) {
        ImGui::LabelText(
            "Pos lights", "%d / %d", static_cast<int>(state.pos_lights.size()),
            MAX_POS_LIGHTS
        );
        ImGui::LabelText(
            "Spot lights", "%d / %d",
            static_cast<int>(state.spot_lights.size()), MAX_SPOT_LIGHTS
        );
        ImGui::LabelText(
            "Flashlight", "%s", state.flashlight_on ? "ON" : "OFF"
        );
    } else {
        int pos_count = static_cast<int>(state.pos_lights.size());
        if (ImGui::SliderInt("Pos lights", &pos_count, 0, MAX_POS_LIGHTS)) {
            while (static_cast<int>(state.pos_lights.size()) < pos_count)
                state.pos_lights.push_back(random_positional_light());
            while (static_cast<int>(state.pos_lights.size()) > pos_count)
                state.pos_lights.pop_back();
        }
        int spot_count = static_cast<int>(state.spot_lights.size());
        if (ImGui::SliderInt("Spot lights", &spot_count, 0, MAX_SPOT_LIGHTS)) {
            while (static_cast<int>(state.spot_lights.size()) < spot_count)
                state.spot_lights.push_back(random_spot_light());
            while (static_cast<int>(state.spot_lights.size()) > spot_count)
                state.spot_lights.pop_back();
        }
        ImGui::Checkbox("Flashlight", &state.flashlight_on);
    }

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);

    ImGui::PopItemWidth();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void set_depth_func() {
    auto &depth_mode = depth_modes[state.depth_mode_index];
    glDepthFunc(depth_mode.mode);
}

void key_callback_combined(
    GLFWwindow *window, int key, int scancode, int action, int mods
) {
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }
    key_callback(window, key, scancode, action, mods);

    if ((key == GLFW_KEY_P) &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (mods & GLFW_MOD_SHIFT) {
            state.preset_index =
                (presets.size() + state.preset_index - 1) % presets.size();
        } else {
            state.preset_index = (state.preset_index + 1) % presets.size();
        }
        init_state();
    }
    if ((key == GLFW_KEY_M) &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (mods & GLFW_MOD_SHIFT) {
            state.depth_mode_index =
                (depth_modes.size() + state.depth_mode_index - 1) %
                depth_modes.size();
        } else {
            state.depth_mode_index =
                (state.depth_mode_index + 1) % depth_modes.size();
        }
        set_depth_func();
    }
    if (key == GLFW_KEY_EQUAL &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if ((mods & GLFW_MOD_SHIFT) &&
            static_cast<int>(state.pos_lights.size()) < MAX_POS_LIGHTS)
            state.pos_lights.push_back(random_positional_light());
    }
    if (key == GLFW_KEY_MINUS &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        if (!state.pos_lights.empty())
            state.pos_lights.pop_back();
    }
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        if (static_cast<int>(state.spot_lights.size()) < MAX_SPOT_LIGHTS)
            state.spot_lights.push_back(random_spot_light());
    }
    if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
        if (!state.spot_lights.empty())
            state.spot_lights.pop_back();
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        state.flashlight_on = !state.flashlight_on;
    }
}

void process_input(GLFWwindow *window, input_t &input) {
    process_common_input(window, input);
}

int main() {
    auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
    if (!ctx) {
        std::cerr << ctx.error() << "\n";
        return -1;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    window_callbacks_t window_callbacks{DEFAULT_WINDOW_CALLBACKS};
    window_callbacks.key = key_callback_combined;
    init_window_callbacks(ctx->window(), state.window, window_callbacks);

    auto renderer_res = SceneRenderer::create(ctx->window());
    if (!renderer_res) {
        std::cerr << renderer_res.error() << "\n";
        return -1;
    }
    auto renderer = std::move(*renderer_res);
    init_state();
    set_depth_func();

    event_loop(ctx->window(), *renderer, process_input);
    return 0;
}
