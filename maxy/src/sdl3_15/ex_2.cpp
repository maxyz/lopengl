#include <array>
#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int        WINDOW_WIDTH     = 1024;
constexpr int        WINDOW_HEIGHT    = 768;
constexpr SDL_FColor BACKGROUND_COLOR = {0.1f, 0.1f, 0.1f, 1.0f};
constexpr float      LIGHT_SPEED      = 2.5f;

struct material_uniforms_t {
    glm::vec4 shininess_pad; // .x = shininess, .yzw unused
};

struct light_uniforms_t {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 position; // camera-relative world space; updated each frame
};

constexpr SDL_GPUVertexAttribute light_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, position))},
};

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t light_pipeline;
    gpu_geometry_t geometry;
    gpu_material_t material;

    camera_t    camera;
    SDL_Window *m_window       = nullptr;
    glm::vec3   light_position = {2.0f, 1.0f, -2.0f};
    // Stored as vec3 so glm::value_ptr() can be passed directly to ColorEdit3 (float[3]).
    // Converted to vec4 when building light_uniforms_t for the GPU push.
    glm::vec3 light_ambient  = {0.2f, 0.2f, 0.2f};
    glm::vec3 light_diffuse  = {0.5f, 0.5f, 0.5f};
    glm::vec3 light_specular = {1.0f, 1.0f, 1.0f};
    float     m_shininess    = 64.0f;
    float     m_time         = 0.0f;
    float     m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    bool      m_ui_mode      = false; // false = fly (camera), true = UI (ImGui)
    bool      m_prev_grave   = false;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_time += in.dt;
    m_aspect_ratio = in.aspect_ratio;

    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;

    // Grave (`) toggles between fly mode (camera control) and UI mode (ImGui interaction).
    bool grave = in.keys[SDL_SCANCODE_GRAVE];
    if (grave && !m_prev_grave) {
        m_ui_mode = !m_ui_mode;
        SDL_SetWindowRelativeMouseMode(m_window, !m_ui_mode);
        if (!m_ui_mode) {
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy); // drain accumulated delta to prevent camera jump
        }
    }
    m_prev_grave = grave;

    if (!m_ui_mode) {
        camera.process_keys(in.keys, in.dt);
        camera.process_mouse(in.dx, in.dy);
        camera.process_scroll(in.scroll);
    }

    float step = LIGHT_SPEED * in.dt;
    if (in.keys[SDL_SCANCODE_I]) light_position.z -= step;
    if (in.keys[SDL_SCANCODE_K]) light_position.z += step;
    if (in.keys[SDL_SCANCODE_J]) light_position.x -= step;
    if (in.keys[SDL_SCANCODE_L]) light_position.x += step;
    if (in.keys[SDL_SCANCODE_Y]) light_position.y += step;
    if (in.keys[SDL_SCANCODE_H]) light_position.y -= step;

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(180.0f);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText(
        "Light", "(%.2f, %.2f, %.2f)", light_position.x, light_position.y, light_position.z
    );
    ImGui::LabelText("Mode", "%s", m_ui_mode ? "UI (` to fly)" : "Fly (` for UI)");
    if (m_ui_mode) {
        ImGui::SeparatorText("Light");
        ImGui::ColorEdit3("Ambient", glm::value_ptr(light_ambient));
        ImGui::ColorEdit3("Diffuse", glm::value_ptr(light_diffuse));
        ImGui::ColorEdit3("Specular", glm::value_ptr(light_specular));
        ImGui::SeparatorText("Material");
        ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 256.0f);
    }
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view       = glm::mat4(glm::mat3(camera.view_matrix()));
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    material_uniforms_t frame_material = {.shininess_pad = {m_shininess, 0.0f, 0.0f, 0.0f}};
    light_uniforms_t    frame_light    = {
        .ambient  = glm::vec4(light_ambient, 0.0f),
        .diffuse  = glm::vec4(light_diffuse, 0.0f),
        .specular = glm::vec4(light_specular, 0.0f),
        .position = glm::vec4(light_position - camera.position, 0.0f),
    };

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, frame_material);
    push_fragment_uniform(cmd, 1, frame_light);

    for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
        float     angle  = m_time * static_cast<float>(i % 3) * 25.0f;
        glm::vec3 camrel = example_cube_positions[i] - camera.position;
        glm::mat4 model  = glm::rotate(
            glm::translate(glm::mat4(1.0f), camrel), glm::radians(angle),
            glm::vec3(1.0f, 0.3f, 0.5f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_pipeline, geometry, material, pass);
    }

    glm::mat4 light_model = glm::scale(
        glm::translate(glm::mat4(1.0f), light_position - camera.position), glm::vec3(0.2f)
    );
    push_vertex_uniform(cmd, 0, light_model);
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, frame_light);
    draw(light_pipeline, geometry, gpu_material_t{}, pass);
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.m_window = engine.window;

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        (std::string(ASSETS_PATH) + "fonts/NotoSans-Regular.ttf").c_str(), 20.0f
    );

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_15/cube.vert.spv",
                    .fragment_shader          = "shaders/sdl3_15/cube_specular.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 2,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!cube) return std::unexpected(cube.error());
    scene.cube_pipeline = std::move(*cube);

    auto light_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_15/light.vert.spv",
                    .fragment_shader          = "shaders/sdl3_15/light_strobe.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = light_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!light_pipe) return std::unexpected(light_pipe.error());
    scene.light_pipeline = std::move(*light_pipe);

    auto geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!geom) return std::unexpected(geom.error());
    scene.geometry = std::move(*geom);

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
    auto result = run_app(
        argc, argv, "SDL3 15 - Ex 2: Adjustable Shininess", WINDOW_WIDTH, WINDOW_HEIGHT,
        BACKGROUND_COLOR, [](engine_t &engine) { return create_scene(engine); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
