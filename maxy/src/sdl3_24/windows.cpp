#include "engine.hpp"
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <print>

constexpr std::string_view TITLE         = "Transparent windows";
constexpr int              WINDOW_WIDTH  = 1024;
constexpr int              WINDOW_HEIGHT = 768;

struct model_placement_t {
    glm::vec3 position;
    float     scale;
};

static constexpr std::array<model_placement_t, 2> CUBES = {{
    {{-1.0f, 0.5f, 1.0f}, 1.0f},
    {{2.0f, 1.0f, 0.0f}, 2.0f},
}};

static constexpr std::array<model_placement_t, 5> WINDOWS = {{
    {{-1.5f, 0.4f, -0.48f}, 0.8f},
    {{1.5f, 0.45f, 0.51f}, 0.9f},
    {{0.0f, 0.5f, 0.7f}, 1.0f},
    {{-0.3f, 0.55f, -2.3f}, 1.1f},
    {{0.5f, 0.6f, -0.6f}, 1.2f},
}};

static constexpr model_placement_t PLANE = {{0.0f, 0.0f, 0.0f}, 1.0f};

struct scene_t {
    gpu_pipeline_t opaque_pipeline;
    gpu_pipeline_t blend_pipeline;
    gpu_geometry_t cube_geometry;
    gpu_geometry_t plane_geometry;
    gpu_geometry_t quad_geometry;
    gpu_material_t marble_material;
    gpu_material_t metal_material;
    gpu_material_t window_material;

    camera_t camera;
    float    m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    auto opaque_pipeline = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_24/grass.vert.spv",
                    .fragment_shader        = "shaders/sdl3_24/grass.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 1,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_depth_test      = true,
                }
    );
    if (!opaque_pipeline) return std::unexpected(opaque_pipeline.error());

    auto blend_pipeline = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_24/grass.vert.spv",
                    .fragment_shader        = "shaders/sdl3_24/grass.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 1,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_depth_test      = true,
                    .enable_blend           = true,
                }
    );
    if (!blend_pipeline) return std::unexpected(blend_pipeline.error());

    auto cube_geometry = create_vertex_geometry(
        engine, unit_cube_with_normals.data(), static_cast<Uint32>(sizeof(unit_cube_with_normals)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!cube_geometry) return std::unexpected(cube_geometry.error());

    auto plane_geometry = create_vertex_geometry(
        engine, floor_plane_vertices.data(), static_cast<Uint32>(sizeof(floor_plane_vertices)),
        static_cast<Uint32>(floor_plane_vertices.size())
    );
    if (!plane_geometry) return std::unexpected(plane_geometry.error());

    auto quad_geometry = create_vertex_geometry(
        engine, vertical_quad_vertices.data(), static_cast<Uint32>(sizeof(vertical_quad_vertices)),
        static_cast<Uint32>(vertical_quad_vertices.size())
    );
    if (!quad_geometry) return std::unexpected(quad_geometry.error());

    auto marble_material = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/marble.jpg"}}
    );
    if (!marble_material) return std::unexpected(marble_material.error());

    auto metal_material = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/metal.png"}}
    );
    if (!metal_material) return std::unexpected(metal_material.error());

    auto window_material = create_material(
        engine, {
                    .texture_paths = {std::string(ASSETS_PATH) + "textures/window.png"},
                    .address_modes = {SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE},
                }
    );
    if (!window_material) return std::unexpected(window_material.error());

    return scene_t{
        .opaque_pipeline = std::move(*opaque_pipeline),
        .blend_pipeline  = std::move(*blend_pipeline),
        .cube_geometry   = std::move(*cube_geometry),
        .plane_geometry  = std::move(*plane_geometry),
        .quad_geometry   = std::move(*quad_geometry),
        .marble_material = std::move(*marble_material),
        .metal_material  = std::move(*metal_material),
        .window_material = std::move(*window_material),
        .camera          = camera_t{engine.window, {0.0f, 0.5f, 3.0f}},
    };
}

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;

    camera.update(in);

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(180.0f);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    auto const view = camera.rotation_view();
    auto const proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    // Opaque geometry: floor and cubes.
    SDL_BindGPUGraphicsPipeline(pass, opaque_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);

    {
        auto model = glm::translate(glm::mat4{1.0f}, PLANE.position - camera.position);
        push_vertex_uniform(cmd, 0, model);
        draw(plane_geometry, metal_material, pass);
    }

    for (auto const &placement : CUBES) {
        auto model = glm::translate(glm::mat4{1.0f}, placement.position - camera.position);
        model      = glm::scale(model, glm::vec3{placement.scale});
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, marble_material, pass);
    }

    // Transparent windows: sort farthest-first so each window blends with the
    // correct background already in the colour buffer.
    std::array<model_placement_t, WINDOWS.size()> sorted = WINDOWS;
    std::ranges::sort(sorted, [&](auto const &a, auto const &b) {
        return glm::length(a.position - camera.position) >
               glm::length(b.position - camera.position);
    });

    SDL_BindGPUGraphicsPipeline(pass, blend_pipeline.get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);

    for (auto const &placement : sorted) {
        auto model = glm::translate(glm::mat4{1.0f}, placement.position - camera.position);
        model      = glm::scale(model, glm::vec3{placement.scale});
        push_vertex_uniform(cmd, 0, model);
        draw(quad_geometry, window_material, pass);
    }
}

int main(int argc, char *argv[]) {
    auto result = run_app(
        argc, argv, TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_FColor{0.01f, 0.01f, 0.0f, 1.0f},
        create_scene
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
