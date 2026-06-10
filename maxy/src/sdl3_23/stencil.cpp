#include "engine.hpp"
#include <SDL3/SDL_main.h>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <print>

constexpr std::string_view TITLE         = "Stencil testing";
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
static constexpr model_placement_t                PLANE = {{0.0f, 0.0f, 0.0f}, 1.0f};

struct scene_t {
    gpu_pipeline_t plane_pipeline;
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t border_pipeline;
    gpu_geometry_t cube_geometry;
    gpu_geometry_t plane_geometry;
    gpu_material_t marble_material;
    gpu_material_t metal_material;

    camera_t camera;
    float    m_border_thickness = 0.005f;
    float    m_aspect_ratio     = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    // Plane pipeline: depth write on, stencil writes disabled -- plane is never outlined.
    auto plane_pipeline = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_23/stencil.vert.spv",
                    .fragment_shader        = "shaders/sdl3_23/stencil.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 1,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_depth_test      = true,
                    .enable_stencil_test    = true,
                    .stencil_write_mask     = 0x00,
                    .stencil_compare_op     = SDL_GPU_COMPAREOP_ALWAYS,
                }
    );
    if (!plane_pipeline) return std::unexpected(plane_pipeline.error());

    // Cube pipeline: depth + stencil writes on; stamps reference value into stencil.
    auto cube_pipeline = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_23/stencil.vert.spv",
                    .fragment_shader        = "shaders/sdl3_23/stencil.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 1,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_depth_test      = true,
                    .enable_stencil_test    = true,
                    .stencil_write_mask     = 0xFF,
                    .stencil_compare_op     = SDL_GPU_COMPAREOP_ALWAYS,
                    .stencil_pass_op        = SDL_GPU_STENCILOP_REPLACE,
                }
    );
    if (!cube_pipeline) return std::unexpected(cube_pipeline.error());

    // Border pipeline: depth test off (outline draws on top), stencil test NOT_EQUAL(1).
    // Only fragments outside the filled cube area (stencil != 1) are drawn.
    auto border_pipeline = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_23/border.vert.spv",
                    .fragment_shader        = "shaders/sdl3_23/border.frag.spv",
                    .vertex_uniform_buffers = 4,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_stencil_test    = true,
                    .stencil_write_mask     = 0x00,
                    .stencil_compare_op     = SDL_GPU_COMPAREOP_NOT_EQUAL,
                }
    );
    if (!border_pipeline) return std::unexpected(border_pipeline.error());

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

    auto marble_material = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/marble.jpg"}}
    );
    if (!marble_material) return std::unexpected(marble_material.error());

    auto metal_material = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/metal.png"}}
    );
    if (!metal_material) return std::unexpected(metal_material.error());

    return scene_t{
        .plane_pipeline  = std::move(*plane_pipeline),
        .cube_pipeline   = std::move(*cube_pipeline),
        .border_pipeline = std::move(*border_pipeline),
        .cube_geometry   = std::move(*cube_geometry),
        .plane_geometry  = std::move(*plane_geometry),
        .marble_material = std::move(*marble_material),
        .metal_material  = std::move(*metal_material),
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
    if (camera.ui_mode()) {
        ImGui::SliderFloat("Border", &m_border_thickness, 0.0f, 0.05f, "%.4f");
    } else {
        ImGui::LabelText("Border", "%.4f", m_border_thickness);
    }
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    auto const view = camera.rotation_view();
    auto const proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    auto draw_cube = [&](model_placement_t const &placement) {
        auto model = glm::translate(glm::mat4{1.0f}, placement.position - camera.position);
        model      = glm::scale(model, glm::vec3{placement.scale});
        push_vertex_uniform(cmd, 0, model);
        SDL_GPUBufferBinding vb = {.buffer = cube_geometry.vertex_buffer.get(), .offset = 0};
        SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);
        SDL_DrawGPUPrimitives(pass, cube_geometry.vertex_count, 1, 0, 0);
    };

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);

    // Pass 1: floor -- depth on, stencil writes disabled.
    SDL_BindGPUGraphicsPipeline(pass, plane_pipeline.get());
    {
        auto model = glm::translate(glm::mat4{1.0f}, PLANE.position - camera.position);
        model      = glm::scale(model, glm::vec3{PLANE.scale});
        push_vertex_uniform(cmd, 0, model);
        draw(plane_geometry, metal_material, pass);
    }

    // Pass 2: cubes -- depth on, stencil writes 1 where drawn.
    SDL_BindGPUGraphicsPipeline(pass, cube_pipeline.get());
    SDL_SetGPUStencilReference(pass, 1);
    SDL_GPUTextureSamplerBinding marble_binding = {
        .texture = marble_material.textures[0].get(), .sampler = marble_material.samplers[0].get()
    };
    SDL_BindGPUFragmentSamplers(pass, 0, &marble_binding, 1);
    for (auto const &cube : CUBES)
        draw_cube(cube);

    // Pass 3: borders -- depth off, stencil test NOT_EQUAL(1), expands cubes in clip space.
    SDL_BindGPUGraphicsPipeline(pass, border_pipeline.get());
    SDL_SetGPUStencilReference(pass, 1);
    push_vertex_uniform(cmd, 3, m_border_thickness);
    for (auto const &cube : CUBES)
        draw_cube(cube);
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
