#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <print>
#include <span>
#include <vector>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int WINDOW_WIDTH  = 1024;
constexpr int WINDOW_HEIGHT = 768;

struct scene_params_t {
    float     shininess;
    int       pos_count;
    int       spot_count;
    int       pad;
    glm::vec4 dir_direction;
    glm::vec4 dir_ambient;
    glm::vec4 dir_diffuse;
    glm::vec4 dir_specular;
};

struct model_placement_t {
    glm::vec3 position;
    float     scale;
};

struct preset_t {
    std::string_view                   name;
    SDL_FColor                         clear_color;
    glm::vec3                          dir_direction;
    glm::vec3                          dir_ambient, dir_diffuse, dir_specular;
    std::vector<pos_light_state_t>     pos_lights;
    std::vector<spot_light_state_t>    spot_lights;
    flashlight_state_t                 flashlight;
    std::span<model_placement_t const> windows;
};

// Far clip is 100 units; +-500 ensures the floor edge is never visible.
// Texture tiles at the same density as floor_plane_vertices (5 units per tile).
static constexpr std::array<pos_normal_uv_vertex_t, 6> large_floor_vertices = {{
    {{500.0f, 0.0f, 500.0f}, {0.0f, 1.0f, 0.0f}, {200.0f, 0.0f}},
    {{-500.0f, 0.0f, -500.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 200.0f}},
    {{-500.0f, 0.0f, 500.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{500.0f, 0.0f, 500.0f}, {0.0f, 1.0f, 0.0f}, {200.0f, 0.0f}},
    {{500.0f, 0.0f, -500.0f}, {0.0f, 1.0f, 0.0f}, {200.0f, 200.0f}},
    {{-500.0f, 0.0f, -500.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 200.0f}},
}};

static constexpr model_placement_t CUBES[] = {
    {{-2.0f, 0.5f, -1.5f}, 1.0f},
    {{2.0f, 1.0f, -2.0f}, 2.0f},
};

static constexpr model_placement_t DESERT_WINDOWS[] = {
    {{-3.5f, 0.5f, 0.0f}, 1.0f},
    {{3.0f, 0.55f, 0.5f}, 1.1f},
    {{0.0f, 0.6f, -0.5f}, 1.2f},
    {{-1.5f, 0.425f, 1.5f}, 0.85f},
};

static constexpr model_placement_t FACTORY_WINDOWS[] = {
    {{-2.0f, 0.4f, -0.5f}, 0.8f}, {{-1.0f, 0.4f, -0.5f}, 0.8f}, {{0.0f, 0.4f, -0.5f}, 0.8f},
    {{1.0f, 0.4f, -0.5f}, 0.8f},  {{2.0f, 0.4f, -0.5f}, 0.8f},
};

static constexpr model_placement_t HORROR_WINDOWS[] = {
    {{0.5f, 0.65f, 1.0f}, 1.3f},
    {{-0.5f, 0.75f, -4.0f}, 1.5f},
};

static constexpr model_placement_t BIOCHEMICAL_WINDOWS[] = {
    {{-0.6f, 0.35f, 0.5f}, 0.7f}, {{0.6f, 0.35f, 0.5f}, 0.7f},   {{-0.6f, 0.35f, -0.3f}, 0.7f},
    {{0.6f, 0.35f, -0.3f}, 0.7f}, {{-0.6f, 0.35f, -1.1f}, 0.7f}, {{0.6f, 0.35f, -1.1f}, 0.7f},
};

const std::array<preset_t, 4>
    PRESETS =
        {
            {
                {
                    .name          = "desert",
                    .clear_color   = {0.75f, 0.52f, 0.3f, 1.0f},
                    .dir_direction = {-0.2f, -1.0f, -0.3f},
                    .dir_ambient   = {0.3f, 0.24f, 0.14f},
                    .dir_diffuse   = {0.7f, 0.42f, 0.26f},
                    .dir_specular  = {0.5f, 0.5f, 0.5f},
                    .pos_lights =
                        {
                            {.position  = {0.0f, 1.5f, 0.0f},
                             .ambient   = {0.1f, 0.06f, 0.0f},
                             .diffuse   = {1.0f, 0.6f, 0.0f},
                             .specular  = {1.0f, 0.6f, 0.0f},
                             .linear    = 0.14f,
                             .quadratic = 0.07f},
                            {.position = {-3.0f, 2.0f, -2.0f},
                             .ambient  = {0.08f, 0.01f, 0.0f},
                             .diffuse  = {0.8f, 0.1f, 0.0f},
                             .specular = {0.8f, 0.1f, 0.0f}},
                        },
                    .spot_lights =
                        {
                            {.position      = {0.0f, 2.0f, 1.0f},
                             .direction     = {0.0f, -1.0f, -0.3f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {0.9f, 0.7f, 0.4f},
                             .specular      = {1.0f, 0.9f, 0.6f},
                             .inner_degrees = 20.0f,
                             .outer_degrees = 27.0f},
                        },
                    .flashlight =
                        {.ambient       = {0.0f, 0.0f, 0.0f},
                         .diffuse       = {0.5f, 0.5f, 0.5f},
                         .specular      = {1.0f, 1.0f, 1.0f},
                         .inner_degrees = 15.0f,
                         .outer_degrees = 20.0f,
                         .constant      = 1.0f,
                         .linear        = 0.09f,
                         .quadratic     = 0.032f},
                    .windows = DESERT_WINDOWS,
                },
                {
                    .name          = "factory",
                    .clear_color   = {0.1f, 0.1f, 0.1f, 1.0f},
                    .dir_direction = {-0.2f, -1.0f, -0.3f},
                    .dir_ambient   = {0.05f, 0.05f, 0.1f},
                    .dir_diffuse   = {0.2f, 0.2f, 0.7f},
                    .dir_specular  = {0.7f, 0.7f, 0.7f},
                    .pos_lights =
                        {
                            {.position = {-2.5f, 2.5f, 0.5f},
                             .ambient  = {0.02f, 0.02f, 0.06f},
                             .diffuse  = {0.2f, 0.2f, 0.6f},
                             .specular = {0.2f, 0.2f, 0.6f}},
                            {.position = {2.5f, 2.5f, 0.5f},
                             .ambient  = {0.02f, 0.02f, 0.06f},
                             .diffuse  = {0.2f, 0.2f, 0.6f},
                             .specular = {0.2f, 0.2f, 0.6f}},
                            {.position = {-2.5f, 2.5f, -1.5f},
                             .ambient  = {0.02f, 0.02f, 0.06f},
                             .diffuse  = {0.2f, 0.2f, 0.6f},
                             .specular = {0.2f, 0.2f, 0.6f}},
                            {.position = {2.5f, 2.5f, -1.5f},
                             .ambient  = {0.02f, 0.02f, 0.06f},
                             .diffuse  = {0.2f, 0.2f, 0.6f},
                             .specular = {0.2f, 0.2f, 0.6f}},
                        },
                    .spot_lights =
                        {
                            {.position      = {-2.0f, 3.0f, -0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {1.0f, 1.0f, 1.0f},
                             .specular      = {1.0f, 1.0f, 1.0f},
                             .inner_degrees = 15.0f,
                             .outer_degrees = 20.0f},
                            {.position      = {0.0f, 3.0f, -0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {1.0f, 1.0f, 1.0f},
                             .specular      = {1.0f, 1.0f, 1.0f},
                             .inner_degrees = 15.0f,
                             .outer_degrees = 20.0f},
                            {.position      = {2.0f, 3.0f, -0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {1.0f, 1.0f, 1.0f},
                             .specular      = {1.0f, 1.0f, 1.0f},
                             .inner_degrees = 15.0f,
                             .outer_degrees = 20.0f},
                        },
                    .flashlight =
                        {.ambient       = {0.0f, 0.0f, 0.0f},
                         .diffuse       = {1.0f, 1.0f, 1.0f},
                         .specular      = {1.0f, 1.0f, 1.0f},
                         .inner_degrees = 15.0f,
                         .outer_degrees = 20.0f,
                         .constant      = 1.0f,
                         .linear        = 0.09f,
                         .quadratic     = 0.032f},
                    .windows = FACTORY_WINDOWS,
                },
                {
                    .name          = "horror",
                    .clear_color   = {0.0f, 0.0f, 0.0f, 1.0f},
                    .dir_direction = {-0.2f, -1.0f, -0.3f},
                    .dir_ambient   = {0.0f, 0.0f, 0.0f},
                    .dir_diffuse   = {0.05f, 0.05f, 0.05f},
                    .dir_specular  = {0.2f, 0.2f, 0.2f},
                    .pos_lights =
                        {
                            {.position  = {0.0f, 1.0f, -3.0f},
                             .ambient   = {0.03f, 0.0f, 0.0f},
                             .diffuse   = {0.3f, 0.0f, 0.0f},
                             .specular  = {0.3f, 0.0f, 0.0f},
                             .linear    = 0.14f,
                             .quadratic = 0.07f},
                        },
                    .spot_lights =
                        {
                            {.position      = {0.0f, 2.5f, 0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {0.15f, 0.15f, 0.15f},
                             .specular      = {0.2f, 0.2f, 0.2f},
                             .inner_degrees = 8.0f,
                             .outer_degrees = 12.0f,
                             .linear        = 0.14f,
                             .quadratic     = 0.07f},
                        },
                    .flashlight =
                        {.ambient       = {0.0f, 0.0f, 0.0f},
                         .diffuse       = {0.5f, 0.5f, 0.5f},
                         .specular      = {1.0f, 1.0f, 1.0f},
                         .inner_degrees = 15.0f,
                         .outer_degrees = 20.0f,
                         .constant      = 1.0f,
                         .linear        = 0.09f,
                         .quadratic     = 0.032f},
                    .windows = HORROR_WINDOWS,
                },
                {
                    .name          = "biochemical",
                    .clear_color   = {0.9f, 0.9f, 0.9f, 1.0f},
                    .dir_direction = {-0.2f, -1.0f, -0.3f},
                    .dir_ambient   = {0.5f, 0.5f, 0.5f},
                    .dir_diffuse   = {1.0f, 1.0f, 1.0f},
                    .dir_specular  = {1.0f, 1.0f, 1.0f},
                    .pos_lights =
                        {
                            {.position = {-3.0f, 2.5f, 0.5f},
                             .ambient  = {0.04f, 0.07f, 0.01f},
                             .diffuse  = {0.4f, 0.7f, 0.1f},
                             .specular = {0.4f, 0.7f, 0.1f}},
                            {.position = {3.0f, 2.5f, 0.5f},
                             .ambient  = {0.04f, 0.07f, 0.01f},
                             .diffuse  = {0.4f, 0.7f, 0.1f},
                             .specular = {0.4f, 0.7f, 0.1f}},
                            {.position = {-3.0f, 2.5f, -1.5f},
                             .ambient  = {0.04f, 0.07f, 0.01f},
                             .diffuse  = {0.4f, 0.7f, 0.1f},
                             .specular = {0.4f, 0.7f, 0.1f}},
                            {.position = {3.0f, 2.5f, -1.5f},
                             .ambient  = {0.04f, 0.07f, 0.01f},
                             .diffuse  = {0.4f, 0.7f, 0.1f},
                             .specular = {0.4f, 0.7f, 0.1f}},
                            {.position  = {0.0f, 3.0f, -0.5f},
                             .ambient   = {0.06f, 0.1f, 0.02f},
                             .diffuse   = {0.6f, 1.0f, 0.2f},
                             .specular  = {0.6f, 1.0f, 0.2f},
                             .linear    = 0.07f,
                             .quadratic = 0.017f},
                        },
                    .spot_lights =
                        {
                            {.position      = {1.0f, 2.5f, -0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {0.0f, 1.0f, 0.0f},
                             .specular      = {0.0f, 1.0f, 0.0f},
                             .inner_degrees = 25.0f,
                             .outer_degrees = 30.0f,
                             .linear        = 0.07f,
                             .quadratic     = 0.017f},
                            {.position      = {-1.0f, 2.5f, -0.5f},
                             .direction     = {0.0f, -1.0f, 0.0f},
                             .ambient       = {0.0f, 0.0f, 0.0f},
                             .diffuse       = {0.0f, 1.0f, 0.0f},
                             .specular      = {0.0f, 1.0f, 0.0f},
                             .inner_degrees = 25.0f,
                             .outer_degrees = 30.0f,
                             .linear        = 0.07f,
                             .quadratic     = 0.017f},
                        },
                    .flashlight =
                        {.ambient       = {0.0f, 0.0f, 0.0f},
                         .diffuse       = {0.0f, 1.0f, 0.0f},
                         .specular      = {0.0f, 1.0f, 0.0f},
                         .inner_degrees = 15.0f,
                         .outer_degrees = 20.0f,
                         .constant      = 1.0f,
                         .linear        = 0.07f,
                         .quadratic     = 0.017f},
                    .windows = BIOCHEMICAL_WINDOWS,
                },
            }
};

struct scene_t {
    gpu_pipeline_t opaque_pipeline;
    gpu_pipeline_t window_back_pipeline;
    gpu_pipeline_t window_front_pipeline;
    gpu_pipeline_t cube_indicator_pipeline;
    gpu_pipeline_t pyramid_indicator_pipeline;

    gpu_geometry_t cube_geometry;
    gpu_geometry_t floor_geometry;
    gpu_geometry_t quad_geometry;
    gpu_geometry_t pyramid_geometry;

    gpu_material_t cube_material;
    gpu_material_t floor_material;
    gpu_material_t window_material;

    camera_t   camera;
    float      m_aspect_ratio  = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    SDL_FColor m_clear_color   = PRESETS[0].clear_color;
    size_t     m_preset_index  = 0;
    bool       m_flashlight_on = true;

    std::vector<pos_light_state_t>  pos_lights;
    std::vector<spot_light_state_t> spot_lights;

    key_edge_t m_p_edge;
    key_edge_t m_plus_edge;
    key_edge_t m_minus_edge;
    key_edge_t m_zero_edge;
    key_edge_t m_nine_edge;
    key_edge_t m_g_edge;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window, {0.0f, 0.5f, 3.0f});
    scene.pos_lights.assign(PRESETS[0].pos_lights.begin(), PRESETS[0].pos_lights.end());
    scene.spot_lights.assign(PRESETS[0].spot_lights.begin(), PRESETS[0].spot_lights.end());

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    auto opaque = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_24/lit.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!opaque) return std::unexpected(opaque.error());
    scene.opaque_pipeline = std::move(*opaque);

    auto window_back = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_24/lit.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                    .enable_depth_write       = false,
                    .cull_mode                = SDL_GPU_CULLMODE_FRONT,
                    .enable_blend             = true,
                }
    );
    if (!window_back) return std::unexpected(window_back.error());
    scene.window_back_pipeline = std::move(*window_back);

    auto window_front = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_24/lit.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                    .enable_depth_write       = false,
                    .cull_mode                = SDL_GPU_CULLMODE_BACK,
                    .enable_blend             = true,
                }
    );
    if (!window_front) return std::unexpected(window_front.error());
    scene.window_front_pipeline = std::move(*window_front);

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

    auto floor_geom = create_vertex_geometry(
        engine, large_floor_vertices.data(),
        static_cast<Uint32>(large_floor_vertices.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(large_floor_vertices.size())
    );
    if (!floor_geom) return std::unexpected(floor_geom.error());
    scene.floor_geometry = std::move(*floor_geom);

    auto quad_geom = create_vertex_geometry(
        engine, vertical_quad_vertices.data(),
        static_cast<Uint32>(vertical_quad_vertices.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(vertical_quad_vertices.size())
    );
    if (!quad_geom) return std::unexpected(quad_geom.error());
    scene.quad_geometry = std::move(*quad_geom);

    auto pyramid_geom = create_geometry(
        engine, pyramid_vertices, sizeof(pyramid_vertices), std::span(pyramid_indices)
    );
    if (!pyramid_geom) return std::unexpected(pyramid_geom.error());
    scene.pyramid_geometry = std::move(*pyramid_geom);

    auto cube_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/container2.png",
                     std::string(ASSETS_PATH) + "textures/container2_specular.png",
                 }}
    );
    if (!cube_mat) return std::unexpected(cube_mat.error());
    scene.cube_material = std::move(*cube_mat);

    auto floor_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/metal.png",
                     std::string(ASSETS_PATH) + "textures/metal.png",
                 }}
    );
    if (!floor_mat) return std::unexpected(floor_mat.error());
    scene.floor_material = std::move(*floor_mat);

    // Window material: window.png diffuse + 1x1 white specular for full specular response.
    auto win_diffuse = load_texture(engine, std::string(ASSETS_PATH) + "textures/window.png");
    if (!win_diffuse) return std::unexpected(win_diffuse.error());

    auto win_specular = create_solid_texture(engine, glm::u8vec4{255, 255, 255, 255});
    if (!win_specular) return std::unexpected(win_specular.error());

    auto sampler_clamp = create_sampler(engine, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
    if (!sampler_clamp) return std::unexpected(sampler_clamp.error());

    auto sampler_repeat = create_sampler(engine);
    if (!sampler_repeat) return std::unexpected(sampler_repeat.error());

    scene.window_material.textures.push_back(std::move(*win_diffuse));
    scene.window_material.textures.push_back(std::move(*win_specular));
    scene.window_material.samplers.push_back(std::move(*sampler_clamp));
    scene.window_material.samplers.push_back(std::move(*sampler_repeat));

    return scene;
}

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

    if (m_g_edge(in.keys[SDL_SCANCODE_G]) && !camera.ui_mode()) m_flashlight_on = !m_flashlight_on;

    if (!camera.ui_mode()) {
        bool plus_key  = shift && in.keys[SDL_SCANCODE_EQUALS];
        bool minus_key = in.keys[SDL_SCANCODE_MINUS];
        bool zero_key  = in.keys[SDL_SCANCODE_0];
        bool nine_key  = in.keys[SDL_SCANCODE_9];

        if (m_plus_edge(plus_key) && static_cast<int>(pos_lights.size()) < MAX_POS_LIGHTS)
            pos_lights.push_back({.position = {0.0f, 1.0f, 0.0f}});
        if (m_minus_edge(minus_key) && !pos_lights.empty()) pos_lights.pop_back();
        if (m_zero_edge(zero_key) && static_cast<int>(spot_lights.size()) < MAX_SPOT_LIGHTS)
            spot_lights.push_back({.position = {0.0f, 2.0f, 0.0f}});
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
    ImGui::LabelText("Flashlight (G)", "%s", m_flashlight_on ? "on" : "off");
    ImGui::LabelText(
        "Pos lights (+/-)", "%d / %d", static_cast<int>(pos_lights.size()), MAX_POS_LIGHTS
    );
    ImGui::LabelText(
        "Spot lights (0/9)", "%d / %d", static_cast<int>(spot_lights.size()), MAX_SPOT_LIGHTS
    );
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view = camera.rotation_view();
    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    auto const &preset = PRESETS[m_preset_index];

    scene_params_t opaque_params = {
        .shininess     = 64.0f,
        .pos_count     = static_cast<int>(pos_lights.size()),
        .spot_count    = static_cast<int>(spot_lights.size()),
        .dir_direction = glm::vec4(preset.dir_direction, 0.0f),
        .dir_ambient   = glm::vec4(preset.dir_ambient, 0.0f),
        .dir_diffuse   = glm::vec4(preset.dir_diffuse, 0.0f),
        .dir_specular  = glm::vec4(preset.dir_specular, 0.0f),
    };

    scene_params_t window_params = opaque_params;
    window_params.shininess      = 128.0f;

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
        .ambient      = m_flashlight_on ? glm::vec4(fl.ambient, 0.0f) : glm::vec4(0.0f),
        .diffuse      = m_flashlight_on ? glm::vec4(fl.diffuse, 0.0f) : glm::vec4(0.0f),
        .specular     = m_flashlight_on ? glm::vec4(fl.specular, 0.0f) : glm::vec4(0.0f),
        .cutoff       = glm::cos(glm::radians(fl.inner_degrees)),
        .outer_cutoff = glm::cos(glm::radians(fl.outer_degrees)),
        .constant     = fl.constant,
        .linear       = fl.linear,
        .quadratic    = fl.quadratic,
    };

    // Opaque geometry: floor and cubes.
    SDL_BindGPUGraphicsPipeline(pass, opaque_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_vertex_uniform(cmd, 3, 1.0f);
    push_fragment_uniform(cmd, 0, opaque_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);

    {
        auto model = glm::translate(glm::mat4{1.0f}, -camera.position);
        push_vertex_uniform(cmd, 0, model);
        draw(floor_geometry, floor_material, pass);
    }

    for (auto const &placement : CUBES) {
        auto model = glm::scale(
            glm::translate(glm::mat4{1.0f}, placement.position - camera.position),
            glm::vec3{placement.scale}
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, cube_material, pass);
    }

    // Light markers.
    SDL_BindGPUGraphicsPipeline(pass, cube_indicator_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    for (auto const &light : pos_lights) {
        push_fragment_uniform(cmd, 0, glm::vec4(light.ambient + light.diffuse, 1.0f));
        auto model = glm::scale(
            glm::translate(glm::mat4{1.0f}, light.position - camera.position), glm::vec3(0.2f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, gpu_material_t{}, pass);
    }

    SDL_BindGPUGraphicsPipeline(pass, pyramid_indicator_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    for (auto const &light : spot_lights) {
        push_fragment_uniform(cmd, 0, glm::vec4(light.ambient + light.diffuse, 1.0f));
        glm::vec3 dir = light.direction;
        glm::vec3 up =
            (std::abs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), dir, up));
        auto model = glm::translate(glm::mat4{1.0f}, light.position - camera.position) * rotation *
                     glm::scale(glm::mat4{1.0f}, glm::vec3(0.2f));
        push_vertex_uniform(cmd, 0, model);
        draw(pyramid_geometry, gpu_material_t{}, pass);
    }

    // Transparent windows: sort farthest-first, then render back faces then front faces so
    // both sides of each pane are lit correctly from any viewing angle.
    std::vector<model_placement_t> sorted(preset.windows.begin(), preset.windows.end());
    std::ranges::sort(sorted, [&](auto const &a, auto const &b) {
        return glm::length(a.position - camera.position) >
               glm::length(b.position - camera.position);
    });

    auto const draw_windows = [&](float normal_flip) {
        push_vertex_uniform(cmd, 3, normal_flip);
        for (auto const &placement : sorted) {
            auto model = glm::scale(
                glm::translate(glm::mat4{1.0f}, placement.position - camera.position),
                glm::vec3{placement.scale}
            );
            push_vertex_uniform(cmd, 0, model);
            draw(quad_geometry, window_material, pass);
        }
    };

    // Back faces: cull front, flip normals inward so inside face is lit correctly.
    SDL_BindGPUGraphicsPipeline(pass, window_back_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_fragment_uniform(cmd, 0, window_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);
    draw_windows(-1.0f);

    // Front faces: cull back, normals outward.
    SDL_BindGPUGraphicsPipeline(pass, window_front_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_fragment_uniform(cmd, 0, window_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);
    draw_windows(1.0f);
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 24 - Multi-light transparent windows", WINDOW_WIDTH, WINDOW_HEIGHT,
        parse_engine_args(argc, argv)
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
