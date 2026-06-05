#include <print>
#include <string>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int        WINDOW_WIDTH     = 800;
constexpr int        WINDOW_HEIGHT    = 600;
constexpr SDL_FColor BACKGROUND_COLOR = {0.2f, 0.3f, 0.3f, 1.0f};
constexpr glm::vec3  LIGHT_POSITION   = {2.0f, 1.0f, -2.0f};

struct lighting_t {
    glm::vec4 object_color;
    glm::vec4 light_color;
};

constexpr lighting_t LIGHTING = {
    .object_color = {1.0f, 0.5f, 0.31f, 0.0f},
    .light_color  = {1.0f, 1.0f, 1.0f, 0.0f},
};

// Light pipeline only reads position (location 0); UVs present in buffer but ignored.
// Pitch stays sizeof(pos_uv_vertex_t) so the GPU advances the same 20 bytes per vertex.
constexpr SDL_GPUVertexAttribute light_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_uv_vertex_t, position))},
};

int main(int argc, char *argv[]) {
    auto config        = parse_engine_args(argc, argv);
    auto engine_result = create_engine("SDL3 12 - Colors", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "{}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    SDL_SetWindowRelativeMouseMode(engine.window, true);

    auto cube_pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_12/colors.vert.spv",
                    .fragment_shader          = "shaders/sdl3_12/colors.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 1,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_uv_buffer_descs,
                    .vertex_attributes        = pos_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!cube_pipeline_result) {
        std::println(stderr, "{}", cube_pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t cube_pipeline = std::move(*cube_pipeline_result);

    auto light_pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_12/light.vert.spv",
                    .fragment_shader        = "shaders/sdl3_12/light.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .vertex_buffer_descs    = pos_uv_buffer_descs,
                    .vertex_attributes      = light_vertex_attributes,
                    .enable_depth_test      = true,
                }
    );
    if (!light_pipeline_result) {
        std::println(stderr, "{}", light_pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t light_pipeline = std::move(*light_pipeline_result);

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

        auto frame = render_frame(
            engine, BACKGROUND_COLOR, depth.texture,
            [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                glm::mat4 view = camera.view_matrix();
                glm::mat4 projection =
                    glm::perspective(glm::radians(camera.fov), aspect_ratio(engine), 0.1f, 100.0f);

                push_vertex_uniform(cmd_buf, 1, view);
                push_vertex_uniform(cmd_buf, 2, projection);
                push_fragment_uniform(cmd_buf, 0, LIGHTING);

                for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
                    float     angle = time * static_cast<float>(i % 3) * 25.0f;
                    glm::mat4 model = glm::rotate(
                        glm::translate(glm::mat4(1.0f), example_cube_positions[i]),
                        glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f)
                    );
                    push_vertex_uniform(cmd_buf, 0, model);
                    draw(cube_pipeline, geometry, material, pass);
                }

                glm::mat4 light_model =
                    glm::scale(glm::translate(glm::mat4(1.0f), LIGHT_POSITION), glm::vec3(0.2f));
                push_vertex_uniform(cmd_buf, 0, light_model);
                push_vertex_uniform(cmd_buf, 1, view);
                push_vertex_uniform(cmd_buf, 2, projection);
                draw(light_pipeline, geometry, gpu_material_t{}, pass);
            }
        );
        if (!frame) {
            std::println(stderr, "{}", frame.error());
            break;
        }
    }
    return 0;
}
