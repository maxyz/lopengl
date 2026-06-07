#include <array>
#include <cmath>
#include <cstdint>
#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int        WINDOW_WIDTH     = 1024;
constexpr int        WINDOW_HEIGHT    = 768;
constexpr SDL_FColor BACKGROUND_COLOR = {0.1f, 0.1f, 0.1f, 1.0f};
constexpr int        NUM_POS_LIGHTS   = 4;
constexpr int        NUM_SPOT_LIGHTS  = 2;

struct material_uniforms_t {
    glm::vec4 shininess_pad; // .x = shininess, .yzw unused
};

constexpr material_uniforms_t MATERIAL = {.shininess_pad = {64.0f, 0.0f, 0.0f, 0.0f}};

struct pos_lights_block_t {
    std::array<positional_light_uniforms_t, NUM_POS_LIGHTS> lights;
};

struct spot_lights_block_t {
    std::array<spot_light_uniforms_t, NUM_SPOT_LIGHTS> lights;
};

// Pyramid: apex at origin, base at z = -1, positions-only (stride = 12 bytes).
constexpr float pyramid_vertices[] = {
    0.0f,  0.0f,  0.0f,  // 0: apex
    -0.5f, 0.5f,  -1.0f, // 1: base
    0.5f,  0.5f,  -1.0f, // 2: base
    0.5f,  -0.5f, -1.0f, // 3: base
    -0.5f, -0.5f, -1.0f, // 4: base
};

constexpr uint16_t pyramid_indices[] = {
    0, 2, 1, // side 1
    0, 3, 2, // side 2
    0, 4, 3, // side 3
    0, 1, 4, // side 4
    1, 2, 3, // base 1
    1, 3, 4, // base 2
};

constexpr SDL_GPUVertexBufferDescription pyramid_buffer_descs[] = {
    {.slot = 0, .pitch = 3 * sizeof(float), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX},
};

constexpr SDL_GPUVertexAttribute pyramid_vertex_attributes[] = {
    {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
};

struct pos_light_state_t {
    glm::vec3 position;
    glm::vec3 ambient  = {0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse  = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct spot_light_state_t {
    glm::vec3 position;
    glm::vec3 direction     = {0.0f, 0.0f, -1.0f};
    glm::vec3 ambient       = {0.0f, 0.0f, 0.0f};
    glm::vec3 diffuse       = {0.5f, 0.5f, 0.5f};
    glm::vec3 specular      = {1.0f, 1.0f, 1.0f};
    float     inner_degrees = 25.0f;
    float     outer_degrees = 30.0f;
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t cube_indicator_pipeline;
    gpu_pipeline_t pyramid_indicator_pipeline;
    gpu_geometry_t cube_geometry;
    gpu_geometry_t pyramid_geometry;
    gpu_material_t material;

    camera_t camera;
    float    m_time         = 0.0f;
    float    m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    glm::vec3 dir_light_direction = {-0.2f, -1.0f, -0.3f};
    glm::vec3 dir_light_ambient   = {0.05f, 0.05f, 0.05f};
    glm::vec3 dir_light_diffuse   = {0.4f, 0.4f, 0.4f};
    glm::vec3 dir_light_specular  = {0.5f, 0.5f, 0.5f};

    std::array<pos_light_state_t, NUM_POS_LIGHTS> pos_lights = {{
        {.position = {0.7f, 0.2f, 2.0f}},
        {.position = {2.3f, -3.3f, -4.0f}},
        {.position = {-4.0f, 2.0f, 12.0f}},
        {.position = {0.0f, 0.0f, -3.0f}},
    }};

    std::array<spot_light_state_t, NUM_SPOT_LIGHTS> spot_lights = {{
        {.position = {1.0f, 1.0f, 0.0f},
         .ambient  = {0.0f, 0.0f, 0.0f},
         .diffuse  = {0.5f, 0.5f, 0.5f}},
        {.position = {-1.0f, 1.0f, 0.0f},
         .ambient  = {0.2f, 0.2f, 0.2f},
         .diffuse  = {0.5f, 0.5f, 0.5f}},
    }};

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_time += in.dt;
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
        ImGui::SeparatorText("Directional light");
        ImGui::DragFloat3("Dir", glm::value_ptr(dir_light_direction), 0.01f, -1.0f, 1.0f);
        ImGui::ColorEdit3("Ambient##dir", glm::value_ptr(dir_light_ambient));
        ImGui::ColorEdit3("Diffuse##dir", glm::value_ptr(dir_light_diffuse));
    }
    ImGui::PopItemWidth();
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view       = camera.rotation_view();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    directional_light_uniforms_t dir_uniform = {
        .direction = glm::vec4(dir_light_direction, 0.0f),
        .ambient   = glm::vec4(dir_light_ambient, 0.0f),
        .diffuse   = glm::vec4(dir_light_diffuse, 0.0f),
        .specular  = glm::vec4(dir_light_specular, 0.0f),
    };

    pos_lights_block_t pos_block;
    for (int i = 0; i < NUM_POS_LIGHTS; ++i) {
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

    spot_lights_block_t spot_block;
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
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
        glm::vec4 color = glm::vec4(pos_lights[i].ambient + pos_lights[i].diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);
        glm::mat4 model = glm::scale(
            glm::translate(glm::mat4(1.0f), pos_lights[i].position - camera.position),
            glm::vec3(0.2f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_indicator_pipeline, cube_geometry, gpu_material_t{}, pass);
    }

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
        glm::vec4 color = glm::vec4(spot_lights[i].ambient + spot_lights[i].diffuse, 1.0f);
        push_fragment_uniform(cmd, 0, color);

        // lookAt maps direction to -Z; inverse gives the model rotation pointing apex toward
        // direction.
        glm::vec3 dir = spot_lights[i].direction;
        glm::vec3 up =
            (std::abs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), dir, up));
        glm::mat4 model =
            glm::translate(glm::mat4(1.0f), spot_lights[i].position - camera.position) * rotation *
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
    auto result = run_app(
        argc, argv, "SDL3 17 - Multiple lights", WINDOW_WIDTH, WINDOW_HEIGHT, BACKGROUND_COLOR,
        [](engine_t &engine) { return create_scene(engine); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
