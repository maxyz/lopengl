#include <array>
#include <cmath>
#include <cstdint>
#include <print>
#include <string_view>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int WINDOW_WIDTH    = 1024;
constexpr int WINDOW_HEIGHT   = 768;
constexpr int NUM_POS_LIGHTS  = 4;
constexpr int NUM_SPOT_LIGHTS = 2;

struct material_uniforms_t {
    glm::vec4 shininess_pad; // .x = shininess, .yzw unused
};

constexpr material_uniforms_t MATERIAL = {.shininess_pad = {64.0f, 0.0f, 0.0f, 0.0f}};


struct preset_pos_light_t {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct preset_spot_light_t {
    glm::vec3 position;
    glm::vec3 direction = {0.0f, 0.0f, -1.0f};
    glm::vec3 ambient   = {0.0f, 0.0f, 0.0f};
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     inner_degrees = 25.0f;
    float     outer_degrees = 30.0f;
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct preset_t {
    std::string_view                                 name;
    SDL_FColor                                       sky_color;
    glm::vec3                                        dir_direction;
    glm::vec3                                        dir_ambient;
    glm::vec3                                        dir_diffuse;
    glm::vec3                                        dir_specular;
    std::array<preset_pos_light_t, NUM_POS_LIGHTS>   pos_lights;
    std::array<preset_spot_light_t, NUM_SPOT_LIGHTS> spot_lights;
};

const std::array<preset_t, 4> presets = {{
    {
        .name          = "desert",
        .sky_color     = {0.75f, 0.52f, 0.30f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.30f, 0.24f, 0.14f},
        .dir_diffuse   = {0.70f, 0.42f, 0.26f},
        .dir_specular  = {0.50f, 0.50f, 0.50f},
        .pos_lights    = {{
            {.position = {0.7f, 0.2f, 2.0f},
             .ambient  = {0.10f, 0.06f, 0.00f},
             .diffuse  = {1.0f, 0.6f, 0.0f},
             .specular = {1.0f, 0.6f, 0.0f}},
            {.position = {2.3f, -3.3f, -4.0f},
             .ambient  = {0.10f, 0.00f, 0.00f},
             .diffuse  = {1.0f, 0.0f, 0.0f},
             .specular = {1.0f, 0.0f, 0.0f}},
            {.position = {-4.0f, 2.0f, 12.0f},
             .ambient  = {0.10f, 0.10f, 0.00f},
             .diffuse  = {1.0f, 1.0f, 0.0f},
             .specular = {1.0f, 1.0f, 0.0f}},
            {.position = {0.0f, 0.0f, -3.0f},
             .ambient  = {0.02f, 0.02f, 0.10f},
             .diffuse  = {0.2f, 0.2f, 1.0f},
             .specular = {0.2f, 0.2f, 1.0f}},
        }},
        .spot_lights   = {{
            {.position = {1.0f, 1.0f, 0.0f},
             .diffuse  = {0.5f, 0.5f, 0.5f},
             .specular = {1.0f, 1.0f, 1.0f}},
            {.position = {-1.0f, 1.0f, 0.0f},
             .diffuse  = {0.5f, 0.5f, 0.5f},
             .specular = {1.0f, 1.0f, 1.0f}},
        }},
    },
    {
        .name          = "factory",
        .sky_color     = {0.10f, 0.10f, 0.10f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.05f, 0.05f, 0.10f},
        .dir_diffuse   = {0.20f, 0.20f, 0.70f},
        .dir_specular  = {0.70f, 0.70f, 0.70f},
        .pos_lights    = {{
            {.position = {0.7f, 0.2f, 2.0f},
             .ambient  = {0.02f, 0.02f, 0.06f},
             .diffuse  = {0.2f, 0.2f, 0.6f},
             .specular = {0.2f, 0.2f, 0.6f}},
            {.position = {2.3f, -3.3f, -4.0f},
             .ambient  = {0.03f, 0.03f, 0.07f},
             .diffuse  = {0.3f, 0.3f, 0.7f},
             .specular = {0.3f, 0.3f, 0.7f}},
            {.position = {-4.0f, 2.0f, 12.0f},
             .ambient  = {0.00f, 0.00f, 0.03f},
             .diffuse  = {0.0f, 0.0f, 0.3f},
             .specular = {0.0f, 0.0f, 0.3f}},
            {.position = {0.0f, 0.0f, -3.0f},
             .ambient  = {0.04f, 0.04f, 0.04f},
             .diffuse  = {0.4f, 0.4f, 0.4f},
             .specular = {0.4f, 0.4f, 0.4f}},
        }},
        .spot_lights   = {{
            {.position = {1.0f, 1.0f, 0.0f},
             .diffuse  = {1.0f, 1.0f, 1.0f},
             .specular = {1.0f, 1.0f, 1.0f}},
            {.position = {-1.0f, 1.0f, 0.0f},
             .diffuse  = {1.0f, 1.0f, 1.0f},
             .specular = {1.0f, 1.0f, 1.0f}},
        }},
    },
    {
        .name          = "horror",
        .sky_color     = {0.00f, 0.00f, 0.00f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.00f, 0.00f, 0.00f},
        .dir_diffuse   = {0.05f, 0.05f, 0.05f},
        .dir_specular  = {0.20f, 0.20f, 0.20f},
        .pos_lights    = {{
            {.position = {0.7f, 0.2f, 2.0f},
             .ambient  = {0.01f, 0.01f, 0.01f},
             .diffuse  = {0.1f, 0.1f, 0.1f},
             .specular = {0.1f, 0.1f, 0.1f}},
            {.position = {2.3f, -3.3f, -4.0f},
             .ambient  = {0.01f, 0.01f, 0.01f},
             .diffuse  = {0.1f, 0.1f, 0.1f},
             .specular = {0.1f, 0.1f, 0.1f}},
            {.position = {-4.0f, 2.0f, 12.0f},
             .ambient  = {0.01f, 0.01f, 0.01f},
             .diffuse  = {0.1f, 0.1f, 0.1f},
             .specular = {0.1f, 0.1f, 0.1f}},
            {.position = {0.0f, 0.0f, -3.0f},
             .ambient  = {0.03f, 0.01f, 0.01f},
             .diffuse  = {0.3f, 0.1f, 0.1f},
             .specular = {0.3f, 0.1f, 0.1f}},
        }},
        .spot_lights   = {{
            {.position = {1.0f, 1.0f, 0.0f},
             .diffuse  = {1.0f, 1.0f, 1.0f},
             .specular = {1.0f, 1.0f, 1.0f}},
            {.position = {-1.0f, 1.0f, 0.0f},
             .diffuse  = {1.0f, 1.0f, 1.0f},
             .specular = {1.0f, 1.0f, 1.0f}},
        }},
    },
    {
        .name          = "biochemical",
        .sky_color     = {0.90f, 0.90f, 0.90f, 1.0f},
        .dir_direction = {-0.2f, -1.0f, -0.3f},
        .dir_ambient   = {0.50f, 0.50f, 0.50f},
        .dir_diffuse   = {1.00f, 1.00f, 1.00f},
        .dir_specular  = {1.00f, 1.00f, 1.00f},
        .pos_lights    = {{
            {.position = {0.7f, 0.2f, 2.0f},
             .ambient  = {0.04f, 0.07f, 0.01f},
             .diffuse  = {0.4f, 0.7f, 0.1f},
             .specular = {0.4f, 0.7f, 0.1f}},
            {.position = {2.3f, -3.3f, -4.0f},
             .ambient  = {0.04f, 0.07f, 0.01f},
             .diffuse  = {0.4f, 0.7f, 0.1f},
             .specular = {0.4f, 0.7f, 0.1f}},
            {.position = {-4.0f, 2.0f, 12.0f},
             .ambient  = {0.04f, 0.07f, 0.01f},
             .diffuse  = {0.4f, 0.7f, 0.1f},
             .specular = {0.4f, 0.7f, 0.1f}},
            {.position = {0.0f, 0.0f, -3.0f},
             .ambient  = {0.04f, 0.07f, 0.01f},
             .diffuse  = {0.4f, 0.7f, 0.1f},
             .specular = {0.4f, 0.7f, 0.1f}},
        }},
        .spot_lights   = {{
            {.position  = {1.0f, 1.0f, 0.0f},
             .diffuse   = {0.0f, 1.0f, 0.0f},
             .specular  = {0.0f, 1.0f, 0.0f},
             .linear    = 0.07f,
             .quadratic = 0.017f},
            {.position  = {-1.0f, 1.0f, 0.0f},
             .diffuse   = {0.0f, 1.0f, 0.0f},
             .specular  = {0.0f, 1.0f, 0.0f},
             .linear    = 0.07f,
             .quadratic = 0.017f},
        }},
    },
}};

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t cube_indicator_pipeline;
    gpu_pipeline_t pyramid_indicator_pipeline;
    gpu_geometry_t cube_geometry;
    gpu_geometry_t pyramid_geometry;
    gpu_material_t material;

    camera_t   camera;
    float      m_time         = 0.0f;
    float      m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    key_edge_t m_p_edge;
    size_t      m_preset_index = 0;
    SDL_FColor  m_clear_color  = presets[0].sky_color;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_time += in.dt;
    m_aspect_ratio = in.aspect_ratio;

    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;

    camera.update(in);

    if (m_p_edge(in.keys[SDL_SCANCODE_P])) {
        bool shift = in.keys[SDL_SCANCODE_LSHIFT] || in.keys[SDL_SCANCODE_RSHIFT];
        if (shift) {
            m_preset_index = (presets.size() + m_preset_index - 1) % presets.size();
        } else {
            m_preset_index = (m_preset_index + 1) % presets.size();
        }
        m_clear_color = presets[m_preset_index].sky_color;
    }

    preset_t const &preset = presets[m_preset_index];

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(180.0f);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Preset", "%zu: %s  (P / Shift+P)", m_preset_index, preset.name.data());
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view       = camera.rotation_view();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    preset_t const &preset = presets[m_preset_index];

    directional_light_uniforms_t dir_uniform = {
        .direction = glm::vec4(preset.dir_direction, 0.0f),
        .ambient   = glm::vec4(preset.dir_ambient, 0.0f),
        .diffuse   = glm::vec4(preset.dir_diffuse, 0.0f),
        .specular  = glm::vec4(preset.dir_specular, 0.0f),
    };

    pos_lights_block_t<NUM_POS_LIGHTS> pos_block;
    for (int i = 0; i < NUM_POS_LIGHTS; ++i) {
        auto const &l       = preset.pos_lights[i];
        pos_block.lights[i] = {
            .position  = glm::vec4(l.position - camera.position, 0.0f),
            .ambient   = glm::vec4(l.ambient, 0.0f),
            .diffuse   = glm::vec4(l.diffuse, 0.0f),
            .specular  = glm::vec4(l.specular, 0.0f),
            .constant  = l.constant,
            .linear    = l.linear,
            .quadratic = l.quadratic,
        };
    }

    spot_lights_block_t<NUM_SPOT_LIGHTS> spot_block;
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
        auto const &s        = preset.spot_lights[i];
        spot_block.lights[i] = {
            .position     = glm::vec4(s.position - camera.position, 0.0f),
            .direction    = glm::vec4(s.direction, 0.0f),
            .ambient      = glm::vec4(s.ambient, 0.0f),
            .diffuse      = glm::vec4(s.diffuse, 0.0f),
            .specular     = glm::vec4(s.specular, 0.0f),
            .cutoff       = glm::cos(glm::radians(s.inner_degrees)),
            .outer_cutoff = glm::cos(glm::radians(s.outer_degrees)),
            .constant     = s.constant,
            .linear       = s.linear,
            .quadratic    = s.quadratic,
        };
    }

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, MATERIAL);
    push_fragment_uniform(cmd, 1, dir_uniform);
    push_fragment_uniform(cmd, 2, pos_block);
    push_fragment_uniform(cmd, 3, spot_block);

    for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
        float     angle  = m_time * static_cast<float>(i % 3) * 25.0f;
        glm::vec3 camrel = example_cube_positions[i] - camera.position;
        glm::mat4 model  = glm::rotate(
            glm::translate(glm::mat4(1.0f), camrel), glm::radians(angle),
            glm::vec3(1.0f, 0.3f, 0.5f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_pipeline, cube_geometry, material, pass);
    }

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    for (int i = 0; i < NUM_POS_LIGHTS; ++i) {
        auto const &l     = preset.pos_lights[i];
        glm::vec4   color = glm::vec4(l.ambient + l.diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.0f), l.position - camera.position), glm::vec3(0.2f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_indicator_pipeline, cube_geometry, gpu_material_t{}, pass);
    }

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
        auto const &s     = preset.spot_lights[i];
        glm::vec4   color = glm::vec4(s.ambient + s.diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);
        glm::vec3 dir = s.direction;
        glm::vec3 up =
            (std::abs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), dir, up));
        glm::mat4 model = glm::translate(glm::mat4(1.0f), s.position - camera.position) * rotation *
                          glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
        push_vertex_uniform(cmd, 0, model);
        draw(pyramid_indicator_pipeline, pyramid_geometry, gpu_material_t{}, pass);
    }
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window);

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        (std::string(ASSETS_PATH) + "fonts/NotoSans-Regular.ttf").c_str(), 20.0f
    );

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_17/cube.vert.spv",
                    .fragment_shader          = "shaders/sdl3_17/multiple.frag.spv",
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
        "SDL3 17 - Presets", WINDOW_WIDTH, WINDOW_HEIGHT, parse_engine_args(argc, argv)
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

    // run_loop with dynamic clear color: background changes when the preset changes.
    auto result = run_loop(
        *engine, [&scene]() { return scene->m_clear_color; },
        [&scene](input_t const &in) { return scene->update(in); },
        [&scene](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) { scene->render(cmd, pass); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
