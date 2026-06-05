// SDL3 port of mouse_input: FPS look via Euler angles + relative mouse mode + scroll zoom.
#include <algorithm>
#include <cmath>
#include <print>
#include <string>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int   WINDOW_WIDTH      = 800;
constexpr int   WINDOW_HEIGHT     = 600;
constexpr float CAM_SPEED         = 3.0f;
constexpr float MOUSE_SENSITIVITY = 0.1f;
constexpr float FOV_SPEED         = 60.0f;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};


// Reconstruct camera_front from Euler angles (spherical to Cartesian).
// yaw starts at -90 so the camera initially looks along -Z.
glm::vec3 camera_front_from_euler(float yaw_deg, float pitch_deg) {
    float yaw   = glm::radians(yaw_deg);
    float pitch = glm::radians(pitch_deg);
    return glm::normalize(
        glm::vec3{
            std::cos(yaw) * std::cos(pitch),
            std::sin(pitch),
            std::sin(yaw) * std::cos(pitch),
        }
    );
}

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("SDL3 10 - Mouse Input", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "{}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    // Capture cursor; SDL3 relative mode delivers per-frame deltas with no first-frame jump.
    SDL_SetWindowRelativeMouseMode(engine.window, true);

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_10/mouse_input.vert.spv",
                    .fragment_shader        = "shaders/sdl3_10/mouse_input.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .fragment_samplers      = 2,
                    .vertex_buffer_descs    = pos_uv_buffer_descs,
                    .vertex_attributes      = pos_uv_vertex_attributes,
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

    constexpr glm::vec3 up         = {0.0f, 1.0f, 0.0f};
    glm::vec3           camera_pos = {0.0f, 0.0f, 3.0f};
    float               yaw        = -90.0f; // -90 so initial front points along -Z
    float               pitch      = 0.0f;
    float               fov        = 45.0f;
    float               time       = 0.0f;

    glm::vec3 camera_front = camera_front_from_euler(yaw, pitch);

    while (true) {
        // Process events inline to capture scroll wheel alongside quit.
        bool      running      = true;
        float     scroll_delta = 0.0f;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_MOUSE_WHEEL) scroll_delta += event.wheel.y;
        }
        if (!running) break;

        float dt = tick(engine);
        time += dt;

        depth.update(engine);

        // Mouse look: SDL3 relative mode gives per-frame deltas; no first-frame guard needed.
        // Negate dy because screen Y increases downward (moving mouse down should look down).
        float dx, dy;
        SDL_GetRelativeMouseState(&dx, &dy);
        yaw += dx * MOUSE_SENSITIVITY;
        pitch -= dy * MOUSE_SENSITIVITY;
        pitch        = std::clamp(pitch, -89.0f, 89.0f);
        camera_front = camera_front_from_euler(yaw, pitch);

        // Scroll zoom.
        fov = std::clamp(fov - scroll_delta, 1.0f, 90.0f);

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        glm::vec3 right = glm::normalize(glm::cross(camera_front, up));
        if (keys[SDL_SCANCODE_W]) camera_pos += CAM_SPEED * dt * camera_front;
        if (keys[SDL_SCANCODE_S]) camera_pos -= CAM_SPEED * dt * camera_front;
        if (keys[SDL_SCANCODE_A]) camera_pos -= CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_D]) camera_pos += CAM_SPEED * dt * right;
        if (keys[SDL_SCANCODE_R]) camera_pos.y += CAM_SPEED * dt;
        if (keys[SDL_SCANCODE_F]) camera_pos.y -= CAM_SPEED * dt;
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
                for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
                    float angle = 20.0f * static_cast<float>(i);
                    if (i % 3 == 0) angle = time * 25.0f;
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), example_cube_positions[i]);
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
