#include <numbers>
#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};
constexpr float      move_speed       = 1.0f; // units per second

constexpr float h = std::numbers::sqrt3_v<float> / 2.0f;

constexpr std::array<colored_vertex_t, 3> triangle = {{
    {{0.5f, -h / 2.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // bottom right - red
    {{-0.5f, -h / 2.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // bottom left  - green
    {{0.0f, h / 2.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},   // top          - blue
}};

constexpr SDL_GPUVertexBufferDescription buffer_descs[] = {{
    .slot       = 0,
    .pitch      = sizeof(colored_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
}};

constexpr SDL_GPUVertexAttribute vertex_attributes[] = {
    {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(colored_vertex_t, color))},
};

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("SDL3 ex 6.8.2 - Offset Uniform", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_06/ex_6_8_2.vert.spv",
                    .fragment_shader        = "shaders/sdl3_06/attributes.frag.spv",
                    .vertex_uniform_buffers = 1,
                    .vertex_buffer_descs    = buffer_descs,
                    .vertex_attributes      = vertex_attributes,
                }
    );
    if (!pipeline_result) {
        std::println(stderr, "Pipeline creation failed: {}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    constexpr Uint32 vertex_data_size =
        static_cast<Uint32>(triangle.size() * sizeof(colored_vertex_t));
    auto buffer_result = create_vertex_buffer(engine, triangle.data(), vertex_data_size);
    if (!buffer_result) {
        std::println(stderr, "Vertex buffer failed: {}", buffer_result.error());
        return 1;
    }
    gpu_buffer_t vertex_buffer = std::move(*buffer_result);

    // In OpenGL the offset state lived inside the shader program and was read
    // back with glGetUniformfv. In SDL3_GPU it lives here on the CPU and is
    // pushed fresh into the command buffer every frame.
    glm::vec2 offset = {0.5f, 0.0f}; // start on the right side

    while (poll_events()) {
        float dt = tick(engine);

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;
        if (keys[SDL_SCANCODE_LEFT]) offset.x -= move_speed * dt;
        if (keys[SDL_SCANCODE_RIGHT]) offset.x += move_speed * dt;
        if (keys[SDL_SCANCODE_UP]) offset.y += move_speed * dt;
        if (keys[SDL_SCANCODE_DOWN]) offset.y -= move_speed * dt;

        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
                SDL_GPUBufferBinding binding = {vertex_buffer.get(), 0};
                SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
                push_vertex_uniform(cmd_buf, 0, offset);
                SDL_DrawGPUPrimitives(pass, triangle.size(), 1, 0, 0);
            }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
