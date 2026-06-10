#include <array>
#include <cmath>
#include <cstdint>
#include <print>
#include <random>
#include <vector>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int WINDOW_WIDTH  = 1024;
constexpr int WINDOW_HEIGHT = 768;

// SDL3 GPU caps fragment uniform buffers at 4, so shininess, directional light,
// and light counts are packed together into one block.
struct scene_params_t {
    float     shininess;
    int       pos_count;
    int       spot_count;
    int       pad; // explicit pad to match std140 layout before the vec4 fields
    glm::vec4 dir_direction;
    glm::vec4 dir_ambient;
    glm::vec4 dir_diffuse;
    glm::vec4 dir_specular;
};

struct preset_t {
    std::string_view                name;
    SDL_FColor                      clear_color;
    glm::vec3                       dir_direction;
    glm::vec3                       dir_ambient, dir_diffuse, dir_specular;
    std::vector<pos_light_state_t>  pos_lights;
    std::vector<spot_light_state_t> spot_lights;
    flashlight_state_t              flashlight;
};

const std::array<preset_t, 4> PRESETS = {{
    {
        .name        = "desert",
        .clear_color = {0.75f, 0.52f, 0.3f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.3f, 0.24f, 0.14f},
        .dir_diffuse   = {0.7f, 0.42f, 0.26f},
        .dir_specular  = {0.5f, 0.5f, 0.5f},
        .pos_lights = {
            {.position={0.7f, 0.2f, 2.0f},   .ambient={0.1f,0.06f,0.0f}, .diffuse={1.0f,0.6f,0.0f}, .specular={1.0f,0.6f,0.0f}},
            {.position={2.3f,-3.3f,-4.0f},   .ambient={0.1f,0.0f,0.0f},  .diffuse={1.0f,0.0f,0.0f}, .specular={1.0f,0.0f,0.0f}},
            {.position={-4.0f,2.0f,12.0f},   .ambient={0.1f,0.1f,0.0f},  .diffuse={1.0f,1.0f,0.0f}, .specular={1.0f,1.0f,0.0f}},
            {.position={0.0f, 0.0f,-3.0f},   .ambient={0.02f,0.02f,0.1f},.diffuse={0.2f,0.2f,1.0f}, .specular={0.2f,0.2f,1.0f}},
        },
        .spot_lights = {
            {.position={1.0f,1.0f,0.0f},  .direction={0.0f,0.0f,-1.0f}},
            {.position={-1.0f,1.0f,0.0f}, .direction={0.0f,0.0f,-1.0f}},
        },
        .flashlight = {.ambient={0.0f,0.0f,0.0f}, .diffuse={0.5f,0.5f,0.5f}, .specular={1.0f,1.0f,1.0f},
                       .inner_degrees=15.0f, .outer_degrees=20.0f, .constant=1.0f, .linear=0.09f, .quadratic=0.032f},
    },
    {
        .name        = "factory",
        .clear_color = {0.1f, 0.1f, 0.1f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.05f, 0.05f, 0.1f},
        .dir_diffuse   = {0.2f, 0.2f, 0.7f},
        .dir_specular  = {0.7f, 0.7f, 0.7f},
        .pos_lights = {
            {.position={0.7f, 0.2f, 2.0f},  .ambient={0.02f,0.02f,0.06f},.diffuse={0.2f,0.2f,0.6f}, .specular={0.2f,0.2f,0.6f}},
            {.position={2.3f,-3.3f,-4.0f},  .ambient={0.03f,0.03f,0.07f},.diffuse={0.3f,0.3f,0.7f}, .specular={0.3f,0.3f,0.7f}},
            {.position={-4.0f,2.0f,12.0f},  .ambient={0.0f,0.0f,0.03f},  .diffuse={0.0f,0.0f,0.3f}, .specular={0.0f,0.0f,0.3f}},
            {.position={0.0f, 0.0f,-3.0f},  .ambient={0.04f,0.04f,0.04f},.diffuse={0.4f,0.4f,0.4f}, .specular={0.4f,0.4f,0.4f}},
        },
        .spot_lights = {
            {.position={1.0f,1.0f,0.0f},   .direction={0.0f,0.0f,-1.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f}},
            {.position={-1.0f,1.0f,0.0f},  .direction={0.0f,0.0f,-1.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f}},
        },
        .flashlight = {.ambient={0.0f,0.0f,0.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f},
                       .inner_degrees=15.0f, .outer_degrees=20.0f, .constant=1.0f, .linear=0.09f, .quadratic=0.032f},
    },
    {
        .name        = "horror",
        .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.0f, 0.0f, 0.0f},
        .dir_diffuse   = {0.05f, 0.05f, 0.05f},
        .dir_specular  = {0.2f, 0.2f, 0.2f},
        .pos_lights = {
            {.position={0.7f, 0.2f, 2.0f},  .ambient={0.01f,0.01f,0.01f},.diffuse={0.1f,0.1f,0.1f}, .specular={0.1f,0.1f,0.1f}},
            {.position={2.3f,-3.3f,-4.0f},  .ambient={0.01f,0.01f,0.01f},.diffuse={0.1f,0.1f,0.1f}, .specular={0.1f,0.1f,0.1f}},
            {.position={-4.0f,2.0f,12.0f},  .ambient={0.01f,0.01f,0.01f},.diffuse={0.1f,0.1f,0.1f}, .specular={0.1f,0.1f,0.1f}},
            {.position={0.0f, 0.0f,-3.0f},  .ambient={0.03f,0.01f,0.01f},.diffuse={0.3f,0.1f,0.1f}, .specular={0.3f,0.1f,0.1f}},
        },
        .spot_lights = {
            {.position={1.0f,1.0f,0.0f},   .direction={0.0f,0.0f,-1.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f}},
            {.position={-1.0f,1.0f,0.0f},  .direction={0.0f,0.0f,-1.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f}},
        },
        .flashlight = {.ambient={0.0f,0.0f,0.0f}, .diffuse={1.0f,1.0f,1.0f}, .specular={1.0f,1.0f,1.0f},
                       .inner_degrees=15.0f, .outer_degrees=20.0f, .constant=1.0f, .linear=0.09f, .quadratic=0.032f},
    },
    {
        .name        = "biochemical",
        .clear_color = {0.9f, 0.9f, 0.9f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.5f, 0.5f, 0.5f},
        .dir_diffuse   = {1.0f, 1.0f, 1.0f},
        .dir_specular  = {1.0f, 1.0f, 1.0f},
        .pos_lights = {
            {.position={0.7f, 0.2f, 2.0f},  .ambient={0.04f,0.07f,0.01f},.diffuse={0.4f,0.7f,0.1f}, .specular={0.4f,0.7f,0.1f}},
            {.position={2.3f,-3.3f,-4.0f},  .ambient={0.04f,0.07f,0.01f},.diffuse={0.4f,0.7f,0.1f}, .specular={0.4f,0.7f,0.1f}},
            {.position={-4.0f,2.0f,12.0f},  .ambient={0.04f,0.07f,0.01f},.diffuse={0.4f,0.7f,0.1f}, .specular={0.4f,0.7f,0.1f}},
            {.position={0.0f, 0.0f,-3.0f},  .ambient={0.04f,0.07f,0.01f},.diffuse={0.4f,0.7f,0.1f}, .specular={0.4f,0.7f,0.1f}},
        },
        .spot_lights ={
            {.position={1.0f,1.0f,0.0f},   .direction={0.0f,0.0f,-1.0f}, .diffuse={0.0f,1.0f,0.0f}, .specular={0.0f,1.0f,0.0f},
             .inner_degrees=25.0f, .outer_degrees=30.0f, .constant=1.0f, .linear=0.07f, .quadratic=0.017f},
            {.position={-1.0f,1.0f,0.0f},  .direction={0.0f,0.0f,-1.0f}, .diffuse={0.0f,1.0f,0.0f}, .specular={0.0f,1.0f,0.0f},
             .inner_degrees=25.0f, .outer_degrees=30.0f, .constant=1.0f, .linear=0.07f, .quadratic=0.017f},
        },
        .flashlight = {.ambient={0.0f,0.0f,0.0f}, .diffuse={0.0f,1.0f,0.0f}, .specular={0.0f,1.0f,0.0f},
                       .inner_degrees=15.0f, .outer_degrees=20.0f, .constant=1.0f, .linear=0.07f, .quadratic=0.017f},
    },
}};

namespace {

std::mt19937 rng{std::random_device{}()};

float rand_float(float lo, float hi) {
    return std::uniform_real_distribution<float>{lo, hi}(rng);
}

pos_light_state_t random_positional_light() {
    glm::vec3 color{rand_float(0.1f, 1.0f), rand_float(0.1f, 1.0f), rand_float(0.1f, 1.0f)};
    return {
        .position  = {rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f)},
        .ambient   = color * 0.1f,
        .diffuse   = color,
        .specular  = color,
        .constant  = 1.0f,
        .linear    = rand_float(0.07f, 0.22f),
        .quadratic = rand_float(0.017f, 0.07f),
    };
}

spot_light_state_t random_spot_light() {
    glm::vec3 color{rand_float(0.1f, 1.0f), rand_float(0.1f, 1.0f), rand_float(0.1f, 1.0f)};
    float     inner = rand_float(10.0f, 25.0f);
    glm::vec3 dir{rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f), rand_float(-1.0f, 1.0f)};
    return {
        .position  = {rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f), rand_float(-5.0f, 5.0f)},
        .direction = glm::normalize(dir),
        .ambient   = color * 0.05f,
        .diffuse   = color,
        .specular  = color,
        .inner_degrees = inner,
        .outer_degrees = inner + rand_float(2.0f, 8.0f),
        .constant      = 1.0f,
        .linear        = rand_float(0.07f, 0.22f),
        .quadratic     = rand_float(0.017f, 0.07f),
    };
}

} // namespace

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t cube_indicator_pipeline;
    gpu_pipeline_t pyramid_indicator_pipeline;
    gpu_geometry_t cube_geometry;
    gpu_geometry_t pyramid_geometry;
    gpu_material_t material;

    camera_t   camera;
    float      m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    SDL_FColor m_clear_color  = PRESETS[0].clear_color;
    size_t     m_preset_index = 0;

    std::vector<pos_light_state_t>  pos_lights;
    std::vector<spot_light_state_t> spot_lights;

    key_edge_t m_p_edge;
    key_edge_t m_plus_edge;
    key_edge_t m_minus_edge;
    key_edge_t m_zero_edge;
    key_edge_t m_nine_edge;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;

    camera.update(in);

    bool shift = in.keys[SDL_SCANCODE_LSHIFT] || in.keys[SDL_SCANCODE_RSHIFT];

    if (m_p_edge(in.keys[SDL_SCANCODE_P]) && !camera.ui_mode()) {
        if (shift)
            m_preset_index = (m_preset_index + PRESETS.size() - 1) % PRESETS.size();
        else
            m_preset_index = (m_preset_index + 1) % PRESETS.size();
        auto const &preset = PRESETS[m_preset_index];
        m_clear_color      = preset.clear_color;
        pos_lights.assign(preset.pos_lights.begin(), preset.pos_lights.end());
        spot_lights.assign(preset.spot_lights.begin(), preset.spot_lights.end());
    }

    if (!camera.ui_mode()) {
        bool plus_key  = shift && in.keys[SDL_SCANCODE_EQUALS];
        bool minus_key = in.keys[SDL_SCANCODE_MINUS];
        bool zero_key  = in.keys[SDL_SCANCODE_0];
        bool nine_key  = in.keys[SDL_SCANCODE_9];

        if (m_plus_edge(plus_key) && static_cast<int>(pos_lights.size()) < MAX_POS_LIGHTS)
            pos_lights.push_back(random_positional_light());
        if (m_minus_edge(minus_key) && !pos_lights.empty()) pos_lights.pop_back();
        if (m_zero_edge(zero_key) && static_cast<int>(spot_lights.size()) < MAX_SPOT_LIGHTS)
            spot_lights.push_back(random_spot_light());
        if (m_nine_edge(nine_key) && !spot_lights.empty()) spot_lights.pop_back();
    }

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(200.0f);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::LabelText(
        "Preset (P / Shift+P)", "%.*s", static_cast<int>(PRESETS[m_preset_index].name.size()),
        PRESETS[m_preset_index].name.data()
    );
    if (camera.ui_mode()) {
        int pos_count = static_cast<int>(pos_lights.size());
        if (ImGui::SliderInt("Pos lights", &pos_count, 0, MAX_POS_LIGHTS)) {
            while (static_cast<int>(pos_lights.size()) < pos_count)
                pos_lights.push_back(random_positional_light());
            while (static_cast<int>(pos_lights.size()) > pos_count)
                pos_lights.pop_back();
        }
        int spot_count = static_cast<int>(spot_lights.size());
        if (ImGui::SliderInt("Spot lights", &spot_count, 0, MAX_SPOT_LIGHTS)) {
            while (static_cast<int>(spot_lights.size()) < spot_count)
                spot_lights.push_back(random_spot_light());
            while (static_cast<int>(spot_lights.size()) > spot_count)
                spot_lights.pop_back();
        }
    } else {
        ImGui::LabelText(
            "Pos lights (+/-)", "%d / %d", static_cast<int>(pos_lights.size()), MAX_POS_LIGHTS
        );
        ImGui::LabelText(
            "Spot lights (0/9)", "%d / %d", static_cast<int>(spot_lights.size()), MAX_SPOT_LIGHTS
        );
    }
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view       = camera.rotation_view();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    auto const &preset = PRESETS[m_preset_index];

    scene_params_t scene_params = {
        .shininess     = 64.0f,
        .pos_count     = static_cast<int>(pos_lights.size()),
        .spot_count    = static_cast<int>(spot_lights.size()),
        .dir_direction = glm::vec4(preset.dir_direction, 0.0f),
        .dir_ambient   = glm::vec4(preset.dir_ambient, 0.0f),
        .dir_diffuse   = glm::vec4(preset.dir_diffuse, 0.0f),
        .dir_specular  = glm::vec4(preset.dir_specular, 0.0f),
    };

    pos_lights_block_t<MAX_POS_LIGHTS> pos_block;
    for (int i = 0; i < static_cast<int>(pos_lights.size()); ++i) {
        pos_block.lights[i] = {
            .position  = glm::vec4(pos_lights[i].position - camera.position, 0.0f),
            .ambient   = glm::vec4(pos_lights[i].ambient, 0.0f),
            .diffuse   = glm::vec4(pos_lights[i].diffuse, 0.0f),
            .specular  = glm::vec4(pos_lights[i].specular, 0.0f),
            .constant  = pos_lights[i].constant,
            .linear    = pos_lights[i].linear,
            .quadratic = pos_lights[i].quadratic,
        };
    }

    spot_lights_block_t<MAX_SPOT_LIGHTS> spot_block;
    for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
        spot_block.lights[i] = {
            .position     = glm::vec4(spot_lights[i].position - camera.position, 0.0f),
            .direction    = glm::vec4(spot_lights[i].direction, 0.0f),
            .ambient      = glm::vec4(spot_lights[i].ambient, 0.0f),
            .diffuse      = glm::vec4(spot_lights[i].diffuse, 0.0f),
            .specular     = glm::vec4(spot_lights[i].specular, 0.0f),
            .cutoff       = glm::cos(glm::radians(spot_lights[i].inner_degrees)),
            .outer_cutoff = glm::cos(glm::radians(spot_lights[i].outer_degrees)),
            .constant     = spot_lights[i].constant,
            .linear       = spot_lights[i].linear,
            .quadratic    = spot_lights[i].quadratic,
        };
    }

    auto const           &fl                 = preset.flashlight;
    flashlight_uniforms_t flashlight_uniform = {
        .direction    = glm::vec4(camera.front(), 0.0f),
        .ambient      = glm::vec4(fl.ambient, 0.0f),
        .diffuse      = glm::vec4(fl.diffuse, 0.0f),
        .specular     = glm::vec4(fl.specular, 0.0f),
        .cutoff       = glm::cos(glm::radians(fl.inner_degrees)),
        .outer_cutoff = glm::cos(glm::radians(fl.outer_degrees)),
        .constant     = fl.constant,
        .linear       = fl.linear,
        .quadratic    = fl.quadratic,
    };

    SDL_BindGPUGraphicsPipeline(pass, cube_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, scene_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);

    for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
        glm::vec3 camrel = example_cube_positions[i] - camera.position;
        glm::mat4 model  = glm::translate(glm::mat4(1.0f), camrel);
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, material, pass);
    }

    SDL_BindGPUGraphicsPipeline(pass, cube_indicator_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    for (int i = 0; i < static_cast<int>(pos_lights.size()); ++i) {
        glm::vec4 color = glm::vec4(pos_lights[i].ambient + pos_lights[i].diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.0f), pos_lights[i].position - camera.position),
            glm::vec3(0.2f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, gpu_material_t{}, pass);
    }

    SDL_BindGPUGraphicsPipeline(pass, pyramid_indicator_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    for (int i = 0; i < static_cast<int>(spot_lights.size()); ++i) {
        glm::vec4 color = glm::vec4(spot_lights[i].ambient + spot_lights[i].diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);
        glm::vec3 dir = spot_lights[i].direction;
        glm::vec3 up =
            (std::abs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        // lookAt maps direction to -Z; inverse gives the model rotation pointing apex toward
        // direction.
        glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), dir, up));
        glm::mat4 model =
            glm::translate(glm::mat4(1.0f), spot_lights[i].position - camera.position) * rotation *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
        push_vertex_uniform(cmd, 0, model);
        draw(pyramid_geometry, gpu_material_t{}, pass);
    }
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window);
    scene.pos_lights.assign(PRESETS[0].pos_lights.begin(), PRESETS[0].pos_lights.end());
    scene.spot_lights.assign(PRESETS[0].spot_lights.begin(), PRESETS[0].spot_lights.end());

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        (std::string(ASSETS_PATH) + "fonts/NotoSans-Regular.ttf").c_str(), 20.0f
    );

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_22/multiple.vert.spv",
                    .fragment_shader          = "shaders/sdl3_22/multiple.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!cube) return std::unexpected(cube.error());
    scene.cube_pipeline = std::move(*cube);

    auto cube_indicator = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_17/light.vert.spv",
                    .fragment_shader          = "shaders/sdl3_17/indicator.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = light_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!cube_indicator) return std::unexpected(cube_indicator.error());
    scene.cube_indicator_pipeline = std::move(*cube_indicator);

    auto pyramid_indicator = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_17/light.vert.spv",
                    .fragment_shader          = "shaders/sdl3_17/indicator.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 1,
                    .vertex_buffer_descs      = pyramid_buffer_descs,
                    .vertex_attributes        = pyramid_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!pyramid_indicator) return std::unexpected(pyramid_indicator.error());
    scene.pyramid_indicator_pipeline = std::move(*pyramid_indicator);

    auto cube_geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!cube_geom) return std::unexpected(cube_geom.error());
    scene.cube_geometry = std::move(*cube_geom);

    auto pyramid_geom = create_geometry(
        engine, pyramid_vertices, sizeof(pyramid_vertices), std::span(pyramid_indices)
    );
    if (!pyramid_geom) return std::unexpected(pyramid_geom.error());
    scene.pyramid_geometry = std::move(*pyramid_geom);

    auto mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/container2.png",
                     std::string(ASSETS_PATH) + "textures/container2_specular.png",
                 }}
    );
    if (!mat) return std::unexpected(mat.error());
    scene.material = std::move(*mat);

    return scene;
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 22 - Multiple Lights", WINDOW_WIDTH, WINDOW_HEIGHT, parse_engine_args(argc, argv)
    );
    if (!engine) {
        std::println(stderr, "{}", engine.error());
        return 1;
    }

    auto scene = create_scene(*engine);
    if (!scene) {
        std::println(stderr, "{}", scene.error());
        return 1;
    }

    // Use the dynamic-clear-color run_loop overload so each preset changes the background.
    auto result = run_loop(
        *engine, [&]() { return scene->m_clear_color; },
        [&](input_t const &in) { return scene->update(in); },
        [&](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) { scene->render(cmd, pass); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
