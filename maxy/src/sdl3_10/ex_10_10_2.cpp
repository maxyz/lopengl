// Exercise 10.10.2: manual LookAt -- build the view matrix from first principles instead of
// glm::lookAt.
#include <print>
#include <string>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};


// Manual LookAt: view = rotation * translation.
//
// Rotation rows are the camera's three axes [right, up, -front].
// GLM mat4 constructor fills columns, so each column gets one component from each axis:
//   col 0: {right.x, up.x, -front.x, 0}
//   col 1: {right.y, up.y, -front.y, 0}  etc.
// -front because OpenGL cameras look along -Z in camera space.
//
// Translation moves the world so the camera is at the origin: T[3] = -position.
// Multiplying rotation * translation gives -dot(axis, pos) in each last-column entry.
glm::mat4 look_at(camera_t const &camera) {
    glm::mat4 rotation{
        camera.right().x,
        camera.up().x,
        -camera.front().x,
        0.0f,
        camera.right().y,
        camera.up().y,
        -camera.front().y,
        0.0f,
        camera.right().z,
        camera.up().z,
        -camera.front().z,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
    };
    glm::mat4 translation{1.0f};
    translation[3][0] = -camera.position.x;
    translation[3][1] = -camera.position.y;
    translation[3][2] = -camera.position.z;
    return rotation * translation;
}

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("SDL3 ex 10.10.2 - Manual LookAt", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "{}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    SDL_SetWindowRelativeMouseMode(engine.window, true);

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_10/ex_10_10_2.vert.spv",
                    .fragment_shader        = "shaders/sdl3_10/ex_10_10_2.frag.spv",
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

    camera_t camera{{0.0f, 0.0f, 3.0f}};
    float    time    = 0.0f;
    bool     focused = true;

    while (true) {
        bool      running      = true;
        float     scroll_delta = 0.0f;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_MOUSE_WHEEL) scroll_delta += event.wheel.y;
            if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
                focused = false;
                SDL_SetWindowRelativeMouseMode(engine.window, false);
            }
            if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
                focused = true;
                SDL_SetWindowRelativeMouseMode(engine.window, true);
            }
        }
        if (!running) break;

        float dt = tick(engine);
        time += dt;

        depth.update(engine);

        if (focused) {
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy);
            camera.process_mouse(dx, -dy);
        }
        camera.process_scroll(scroll_delta);

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;
        camera.process_keys(keys, dt);

        glm::mat4 view = look_at(camera);
        glm::mat4 projection =
            glm::perspective(glm::radians(camera.fov), aspect_ratio(engine), 0.1f, 100.0f);

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
