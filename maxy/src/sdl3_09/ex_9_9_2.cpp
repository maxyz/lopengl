// Exercise 9.9.2: walk around the scene with WASD + R/F (up/down) + Q/E (yaw) + arrows (FOV).
#include <algorithm>
#include <array>
#include <print>
#include <string>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int   WINDOW_WIDTH  = 800;
constexpr int   WINDOW_HEIGHT = 600;
constexpr float CAM_SPEED     = 2.5f;  // units per second
constexpr float YAW_SPEED     = 1.0f;  // scale factor for yaw strafe
constexpr float FOV_SPEED     = 60.0f; // degrees per second

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

constexpr std::array<glm::vec3, 10> cube_positions = {{
    {0.0f, 0.0f, 0.0f},
    {2.0f, 5.0f, -15.0f},
    {-1.5f, -2.2f, -2.5f},
    {-3.8f, -2.0f, -12.3f},
    {2.4f, -0.4f, -3.5f},
    {-1.7f, 3.0f, -7.5f},
    {1.3f, -2.0f, -2.5f},
    {1.5f, 2.0f, -2.5f},
    {1.5f, 0.2f, -1.5f},
    {-1.3f, 1.0f, -1.5f},
}};

constexpr SDL_GPUVertexBufferDescription buffer_descs[]      = {{
    .slot       = 0,
    .pitch      = sizeof(pos_uv_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
}};
constexpr SDL_GPUVertexAttribute         vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_uv_vertex_t, position))},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
     .offset      = static_cast<Uint32>(offsetof(pos_uv_vertex_t, uv))},
};

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("SDL3 ex 9.9.2 - Camera", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "{}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_09/going_3d.vert.spv",
                    .fragment_shader        = "shaders/sdl3_09/going_3d.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 2,
                    .vertex_buffer_descs    = buffer_descs,
                    .vertex_attributes      = vertex_attributes,
                    .enable_depth_test      = true,
                }
    );
    if (!pipeline_result) {
        std::println(stderr, "{}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    auto geometry_result = create_vertex_geometry(
        engine, unit_cube.data(), static_cast<Uint32>(unit_cube.size() * sizeof(pos_uv_vertex_t)),
        static_cast<Uint32>(unit_cube.size())
    );
    if (!geometry_result) {
        std::println(stderr, "{}", geometry_result.error());
        return 1;
    }
    gpu_geometry_t geometry = std::move(*geometry_result);

    auto material_result = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/container.jpg",
                     std::string(ASSETS_PATH) + "textures/awesomeface.png",
                 }}
    );
    if (!material_result) {
        std::println(stderr, "{}", material_result.error());
        return 1;
    }
    gpu_material_t material = std::move(*material_result);

    auto depth_result = create_tracked_depth(engine);
    if (!depth_result) {
        std::println(stderr, "{}", depth_result.error());
        return 1;
    }
    tracked_depth_t depth = std::move(*depth_result);

    constexpr glm::vec3 up           = {0.0f, 1.0f, 0.0f};
    glm::vec3           camera_pos   = {0.0f, 0.0f, 3.0f};
    glm::vec3           camera_front = {0.0f, 0.0f, -1.0f};
    float               fov          = 45.0f;

    while (poll_events()) {
        float dt = tick(engine);

        depth.update(engine);

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        glm::vec3 right = glm::normalize(glm::cross(camera_front, up));
        if (keys[SDL_SCANCODE_W]) camera_pos += CAM_SPEED * dt * camera_front;
        if (keys[SDL_SCANCODE_S]) camera_pos -= CAM_SPEED * dt * camera_front;
        if (keys[SDL_SCANCODE_A]) camera_pos -= CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_D]) camera_pos += CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_R]) camera_pos.y += CAM_SPEED * dt;
        if (keys[SDL_SCANCODE_F]) camera_pos.y -= CAM_SPEED * dt;
        if (keys[SDL_SCANCODE_Q]) camera_front -= YAW_SPEED * CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_E]) camera_front += YAW_SPEED * CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_UP]) fov = std::clamp(fov - FOV_SPEED * dt, 1.0f, 90.0f);
        if (keys[SDL_SCANCODE_DOWN]) fov = std::clamp(fov + FOV_SPEED * dt, 1.0f, 90.0f);

        glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, up);
        glm::mat4 projection =
            glm::perspective(glm::radians(fov), aspect_ratio(engine), 0.1f, 100.0f);

        auto frame = render_frame(
            engine, background_color, depth.texture,
            [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                push_vertex_uniform(cmd_buf, 1, view);
                push_vertex_uniform(cmd_buf, 2, projection);
                for (Uint32 i = 0; i < cube_positions.size(); ++i) {
                    float     angle = 20.0f * static_cast<float>(i);
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), cube_positions[i]);
                    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                    push_vertex_uniform(cmd_buf, 0, model);
                    draw(pipeline, geometry, material, pass);
                }
            }
        );
        if (!frame) {
            std::println(stderr, "{}", frame.error());
            break;
        }
    }
    return 0;
}
