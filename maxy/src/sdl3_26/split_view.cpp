#include <algorithm>
#include <array>
#include <cmath>
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

// Fullscreen quad: SDL3 GPU uses OpenGL-style NDC (y=+1 = top), texture (0,0) = top-left.
struct screen_vertex_t {
    glm::vec2 position;
    glm::vec2 tex_coord;
};

constexpr std::array<screen_vertex_t, 6> SCREEN_QUAD = {{
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},  // top-left
    {{1.0f, 1.0f}, {1.0f, 0.0f}},   // top-right
    {{1.0f, -1.0f}, {1.0f, 1.0f}},  // bottom-right
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},  // top-left
    {{1.0f, -1.0f}, {1.0f, 1.0f}},  // bottom-right
    {{-1.0f, -1.0f}, {0.0f, 1.0f}}, // bottom-left
}};

constexpr SDL_GPUVertexBufferDescription screen_buffer_desc = {
    .slot       = 0,
    .pitch      = sizeof(screen_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
};

constexpr SDL_GPUVertexAttribute screen_vertex_attributes[] = {
    {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .offset = 0},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
     .offset      = static_cast<Uint32>(offsetof(screen_vertex_t, tex_coord))},
};

constexpr std::array<const char *, 5> KERNEL_NAMES = {"None", "Blur", "Edge", "Sharpen", "Emboss"};

// PiP transform: 30% scale, shifted to upper-center of screen.
const glm::mat4 PIP_TRANSFORM = glm::scale(
    glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.8f, 0.0f}), glm::vec3{0.3f, 0.3f, 1.0f}
);

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t floor_pipeline;
    gpu_pipeline_t window_back_pipeline;
    gpu_pipeline_t window_front_pipeline;
    gpu_pipeline_t cube_indicator_pipeline;
    gpu_pipeline_t pyramid_indicator_pipeline;
    gpu_pipeline_t composite_pipeline; // draws a screen quad with a transform

    gpu_geometry_t cube_geometry;
    gpu_geometry_t floor_geometry;
    gpu_geometry_t quad_geometry;
    gpu_geometry_t pyramid_geometry;
    gpu_geometry_t screen_geometry;

    gpu_material_t cube_material;
    gpu_material_t floor_material;
    gpu_material_t window_material;
    gpu_sampler_t  screen_sampler;

    tracked_color_target_t mirror_color_target;
    tracked_color_target_t scene_color_target;

    camera_t   camera;
    float      m_aspect_ratio  = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    SDL_FColor m_clear_color   = PRESETS[0].clear_color;
    size_t     m_preset_index  = 0;
    bool       m_flashlight_on = true;
    uint32_t   m_kernel_idx    = 0;
    bool       m_invert_on     = false;
    bool       m_grey_on       = false;

    std::vector<pos_light_state_t>  pos_lights;
    std::vector<spot_light_state_t> spot_lights;

    key_edge_t m_p_edge;
    key_edge_t m_g_edge;
    key_edge_t m_k_edge;
    key_edge_t m_i_edge;
    key_edge_t m_v_edge;
    key_edge_t m_plus_edge;
    key_edge_t m_minus_edge;
    key_edge_t m_zero_edge;
    key_edge_t m_nine_edge;

    bool update(input_t const &in);

    void draw_world(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::mat4 const &view,
        glm::mat4 const &proj
    );
    void render_mirror(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
    void render_main(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
    void render_composite(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window, {0.0f, 0.5f, 3.0f});
    scene.pos_lights.assign(PRESETS[0].pos_lights.begin(), PRESETS[0].pos_lights.end());
    scene.spot_lights.assign(PRESETS[0].spot_lights.begin(), PRESETS[0].spot_lights.end());

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    auto cube_pipeline = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_24/lit.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                    .cull_mode                = SDL_GPU_CULLMODE_BACK,
                }
    );
    if (!cube_pipeline) return std::unexpected(cube_pipeline.error());
    scene.cube_pipeline = std::move(*cube_pipeline);

    auto floor_pipeline = create_pipeline(
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
    if (!floor_pipeline) return std::unexpected(floor_pipeline.error());
    scene.floor_pipeline = std::move(*floor_pipeline);

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

    // Composite pipeline: no depth attachment; draws screen quads in the final pass.
    auto composite_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_26/overlay.vert.spv",
                    .fragment_shader          = "shaders/sdl3_26/screen.frag.spv",
                    .vertex_uniform_buffers   = 1,
                    .fragment_uniform_buffers = 1,
                    .fragment_samplers        = 1,
                    .vertex_buffer_descs      = std::span(&screen_buffer_desc, 1),
                    .vertex_attributes        = screen_vertex_attributes,
                }
    );
    if (!composite_pipe) return std::unexpected(composite_pipe.error());
    scene.composite_pipeline = std::move(*composite_pipe);

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

    auto screen_geom = create_vertex_geometry(
        engine, SCREEN_QUAD.data(),
        static_cast<Uint32>(SCREEN_QUAD.size() * sizeof(screen_vertex_t)),
        static_cast<Uint32>(SCREEN_QUAD.size())
    );
    if (!screen_geom) return std::unexpected(screen_geom.error());
    scene.screen_geometry = std::move(*screen_geom);

    auto cube_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/metal.png",
                     std::string(ASSETS_PATH) + "textures/metal.png",
                 }}
    );
    if (!cube_mat) return std::unexpected(cube_mat.error());
    scene.cube_material = std::move(*cube_mat);

    auto floor_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/marble.jpg",
                     std::string(ASSETS_PATH) + "textures/marble.jpg",
                 }}
    );
    if (!floor_mat) return std::unexpected(floor_mat.error());
    scene.floor_material = std::move(*floor_mat);

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

    auto screen_samp = create_sampler(engine, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
    if (!screen_samp) return std::unexpected(screen_samp.error());
    scene.screen_sampler = std::move(*screen_samp);

    auto mirror_target = create_tracked_color_target(engine);
    if (!mirror_target) return std::unexpected(mirror_target.error());
    scene.mirror_color_target = std::move(*mirror_target);

    auto scene_target = create_tracked_color_target(engine);
    if (!scene_target) return std::unexpected(scene_target.error());
    scene.scene_color_target = std::move(*scene_target);

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

    if (m_k_edge(in.keys[SDL_SCANCODE_K]) && !camera.ui_mode())
        m_kernel_idx = (m_kernel_idx + 1) % static_cast<uint32_t>(KERNEL_NAMES.size());
    if (m_i_edge(in.keys[SDL_SCANCODE_I]) && !camera.ui_mode()) m_invert_on = !m_invert_on;
    if (m_v_edge(in.keys[SDL_SCANCODE_V]) && !camera.ui_mode()) m_grey_on = !m_grey_on;
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
    ImGui::Separator();
    ImGui::LabelText("Scene kernel (K)", "%s", KERNEL_NAMES[m_kernel_idx]);
    ImGui::LabelText("Scene invert (I)", "%s", m_invert_on ? "on" : "off");
    ImGui::LabelText("Scene greyscale (V)", "%s", m_grey_on ? "on" : "off");
    ImGui::Separator();
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

void scene_t::draw_world(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::mat4 const &view,
    glm::mat4 const &proj
) {
    auto const &preset = PRESETS[m_preset_index];

    scene_params_t opaque_params = {
        .shininess     = 32.0f,
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

    SDL_BindGPUGraphicsPipeline(pass, floor_pipeline.get());
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

    SDL_BindGPUGraphicsPipeline(pass, cube_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_vertex_uniform(cmd, 3, 1.0f);
    push_fragment_uniform(cmd, 0, opaque_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);
    for (auto const &placement : CUBES) {
        auto model = glm::scale(
            glm::translate(glm::mat4{1.0f}, placement.position - camera.position),
            glm::vec3{placement.scale}
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, cube_material, pass);
    }

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

    SDL_BindGPUGraphicsPipeline(pass, window_back_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_fragment_uniform(cmd, 0, window_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);
    draw_windows(-1.0f);

    SDL_BindGPUGraphicsPipeline(pass, window_front_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_fragment_uniform(cmd, 0, window_params);
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_block);
    push_fragment_uniform(cmd, 3, flashlight_uniform);
    draw_windows(1.0f);
}

void scene_t::render_mirror(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    float     mirror_yaw   = camera.yaw + 180.0f;
    float     mirror_pitch = -camera.pitch;
    glm::vec3 front;
    front.x = std::cos(glm::radians(mirror_yaw)) * std::cos(glm::radians(mirror_pitch));
    front.y = std::sin(glm::radians(mirror_pitch));
    front.z = std::sin(glm::radians(mirror_yaw)) * std::cos(glm::radians(mirror_pitch));
    front   = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3{0.0f, 1.0f, 0.0f}));
    glm::vec3 up    = glm::normalize(glm::cross(right, front));

    glm::mat4 view =
        glm::mat4(glm::mat3(glm::lookAt(camera.position, camera.position + front, up)));
    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);
    draw_world(cmd, pass, view, proj);
}

void scene_t::render_main(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view = camera.rotation_view();
    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);
    draw_world(cmd, pass, view, proj);
}

void scene_t::render_composite(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    uint32_t scene_flags =
        m_kernel_idx | (m_invert_on ? 0x8u : 0u) | (m_grey_on ? 0x10u : 0u);
    uint32_t mirror_flags = scene_flags;

    SDL_BindGPUGraphicsPipeline(pass, composite_pipeline.get());

    SDL_GPUBufferBinding vb = {screen_geometry.vertex_buffer.get(), 0};
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

    SDL_GPUTextureSamplerBinding scene_binding = {
        scene_color_target.texture.get(), screen_sampler.get()
    };
    SDL_BindGPUFragmentSamplers(pass, 0, &scene_binding, 1);
    push_vertex_uniform(cmd, 0, glm::mat4{1.0f});
    push_fragment_uniform(cmd, 0, scene_flags);
    SDL_DrawGPUPrimitives(pass, screen_geometry.vertex_count, 1, 0, 0);

    SDL_GPUTextureSamplerBinding mirror_binding = {
        mirror_color_target.texture.get(), screen_sampler.get()
    };
    SDL_BindGPUFragmentSamplers(pass, 0, &mirror_binding, 1);
    push_vertex_uniform(cmd, 0, PIP_TRANSFORM);
    push_fragment_uniform(cmd, 0, mirror_flags);
    SDL_DrawGPUPrimitives(pass, screen_geometry.vertex_count, 1, 0, 0);
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 26 - Split View", WINDOW_WIDTH, WINDOW_HEIGHT, parse_engine_args(argc, argv)
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

    auto depth = create_tracked_depth(*engine);
    if (!depth) {
        std::println(stderr, "{}", depth.error());
        return 1;
    }

    std::array<pass_desc_t, 3> passes = {{
        {.color_target  = &scene->mirror_color_target.texture,
         .depth_texture = &depth->texture,
         .draw          = [&](auto cmd, auto pass) { scene->render_mirror(cmd, pass); }},
        {.color_target  = &scene->scene_color_target.texture,
         .depth_texture = &depth->texture,
         .prepare =
             [&](auto cmd) {
                 scene->scene_color_target.update(*engine);
                 imgui_prepare(cmd);
             },
         .draw = [&](auto cmd, auto pass) { scene->render_main(cmd, pass); }},
        {.draw = [&](auto cmd, auto pass) {
            scene->render_composite(cmd, pass);
            imgui_render(cmd, pass);
        }},
    }};

    auto result = run_loop(
        *engine, [&]() { return scene->m_clear_color; }, *depth, scene->mirror_color_target,
        [&](input_t const &in) { return scene->update(in); }, passes
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
