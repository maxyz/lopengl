#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int        WINDOW_WIDTH     = 1024;
constexpr int        WINDOW_HEIGHT    = 768;
constexpr SDL_FColor BACKGROUND_COLOR = {0.01f, 0.01f, 0.0f, 1.0f};

struct depth_mode_t {
    std::string_view name;
    SDL_GPUCompareOp compare_op;
};

// Ordered to match the original chapter: index 2 = Less (the normal default).
constexpr std::array<depth_mode_t, 8> DEPTH_MODES = {{
    {"Always", SDL_GPU_COMPAREOP_ALWAYS},
    {"Never", SDL_GPU_COMPAREOP_NEVER},
    {"Less", SDL_GPU_COMPAREOP_LESS},
    {"Equal", SDL_GPU_COMPAREOP_EQUAL},
    {"Less or Equal", SDL_GPU_COMPAREOP_LESS_OR_EQUAL},
    {"Greater", SDL_GPU_COMPAREOP_GREATER},
    {"Not Equal", SDL_GPU_COMPAREOP_NOT_EQUAL},
    {"Greater or Equal", SDL_GPU_COMPAREOP_GREATER_OR_EQUAL},
}};

struct object_t {
    glm::vec3 position;
    float     scale;
};

constexpr std::array<object_t, 2> CUBES = {{
    {{-1.0f, 0.5f, -1.0f}, 1.0f},
    {{2.0f, 1.0f, 0.0f}, 2.0f},
}};
constexpr object_t PLANE = {{0.0f, 0.0f, 0.0f}, 1.0f};

struct depth_range_t {
    float near;
    float far;
};

struct scene_t {
    std::array<gpu_pipeline_t, DEPTH_MODES.size()> pipelines;
    gpu_geometry_t                                 cube_geometry;
    gpu_geometry_t                                 plane_geometry;
    gpu_material_t                                 marble_material;
    gpu_material_t                                 metal_material;

    camera_t camera;
    size_t   depth_mode_index = 2;
    key_edge_t m_m_edge;
    float    m_aspect_ratio   = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float    near_plane       = 0.1f;
    float    far_plane        = 100.0f;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;

    camera.update(in);

    if (m_m_edge(in.keys[SDL_SCANCODE_M])) depth_mode_index = (depth_mode_index + 1) % DEPTH_MODES.size();

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(200.0f);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::SeparatorText("Depth Linearization");
    ImGui::SliderFloat("Near", &near_plane, 0.001f, 10.0f, "%.3f");
    ImGui::SliderFloat("Far", &far_plane, 1.0f, 200.0f, "%.1f");
    ImGui::SeparatorText("Depth Test");
    ImGui::LabelText("M key", "cycle mode");
    if (ImGui::BeginCombo("Compare op", DEPTH_MODES[depth_mode_index].name.data())) {
        for (size_t i = 0; i < DEPTH_MODES.size(); ++i) {
            bool selected = (i == depth_mode_index);
            if (ImGui::Selectable(DEPTH_MODES[i].name.data(), selected)) depth_mode_index = i;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view       = camera.rotation_view();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    SDL_BindGPUGraphicsPipeline(pass, pipelines[depth_mode_index].get());
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, depth_range_t{near_plane, far_plane});

    for (auto const &cube : CUBES) {
        glm::mat4 model =
            glm::scale(glm::translate(glm::mat4(1.0f), cube.position - camera.position), glm::vec3(cube.scale));
        push_vertex_uniform(cmd, 0, model);
        draw(cube_geometry, marble_material, pass);
    }

    glm::mat4 plane_model =
        glm::scale(glm::translate(glm::mat4(1.0f), PLANE.position - camera.position), glm::vec3(PLANE.scale));
    push_vertex_uniform(cmd, 0, plane_model);
    draw(plane_geometry, metal_material, pass);
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window, {0.0f, 0.5f, 3.0f});

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        (std::string(ASSETS_PATH) + "fonts/NotoSans-Regular.ttf").c_str(), 20.0f
    );

    for (size_t i = 0; i < DEPTH_MODES.size(); ++i) {
        auto pipe = create_pipeline(
            engine, {
                        .vertex_shader            = "shaders/sdl3_22/linear.vert.spv",
                        .fragment_shader          = "shaders/sdl3_22/linear.frag.spv",
                        .vertex_uniform_buffers   = 3,
                        .fragment_uniform_buffers = 1,
                        .fragment_samplers        = 1,
                        .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                        .vertex_attributes        = pos_normal_uv_vertex_attributes,
                        .enable_depth_test        = true,
                        .depth_compare_op         = DEPTH_MODES[i].compare_op,
                    }
        );
        if (!pipe) return std::unexpected(pipe.error());
        scene.pipelines[i] = std::move(*pipe);
    }

    auto cube_geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!cube_geom) return std::unexpected(cube_geom.error());
    scene.cube_geometry = std::move(*cube_geom);

    auto plane_geom = create_vertex_geometry(
        engine, floor_plane_vertices.data(),
        static_cast<Uint32>(floor_plane_vertices.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(floor_plane_vertices.size())
    );
    if (!plane_geom) return std::unexpected(plane_geom.error());
    scene.plane_geometry = std::move(*plane_geom);

    auto marble = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/marble.jpg"}}
    );
    if (!marble) return std::unexpected(marble.error());
    scene.marble_material = std::move(*marble);

    auto metal = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/metal.png"}}
    );
    if (!metal) return std::unexpected(metal.error());
    scene.metal_material = std::move(*metal);

    return scene;
}

int main(int argc, char *argv[]) {
    auto result = run_app(
        argc, argv, "SDL3 22 - Linear Depth", WINDOW_WIDTH, WINDOW_HEIGHT, BACKGROUND_COLOR,
        [](engine_t &engine) { return create_scene(engine); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
