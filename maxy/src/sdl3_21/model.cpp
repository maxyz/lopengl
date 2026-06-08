#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "model.hpp"

constexpr int        WINDOW_WIDTH     = 1024;
constexpr int        WINDOW_HEIGHT    = 768;
constexpr SDL_FColor BACKGROUND_COLOR = {0.52f, 0.39f, 0.27f, 1.0f}; // desert sand

struct scene_t {
    gpu_pipeline_t pipeline;
    gpu_model_t    model;
    camera_t       camera;
    float          m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.update(in);

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::LabelText("Meshes", "%zu", model.meshes.size());
    ImGui::LabelText("Textures", "%zu", model.textures.size());
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 model_mat  = glm::mat4(1.0f);
    glm::mat4 view       = camera.view_matrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
    push_vertex_uniform(cmd, 0, model_mat);
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    draw_model(model, {texture_slot_t::diffuse}, pass);
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

    auto pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_21/model.vert.spv",
                    .fragment_shader          = "shaders/sdl3_21/model.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 0,
                    .fragment_samplers        = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!pipe) return std::unexpected(pipe.error());
    scene.pipeline = std::move(*pipe);

    auto model = load_model(engine, std::string(ASSETS_PATH) + "objects/backpack/backpack.obj");
    if (!model) return std::unexpected(model.error());
    scene.model = std::move(*model);

    return scene;
}

int main(int argc, char *argv[]) {
    auto result = run_app(
        argc, argv, "SDL3 21 - Model Loading", WINDOW_WIDTH, WINDOW_HEIGHT, BACKGROUND_COLOR,
        [](engine_t &engine) { return create_scene(engine); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
