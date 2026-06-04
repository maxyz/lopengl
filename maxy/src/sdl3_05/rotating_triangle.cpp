#include <numbers>
#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

constexpr auto triangle_vertices = make_equilateral_triangle(0.5f);

constexpr float rotation_rpm       = 1.0f;
constexpr float radians_per_second = rotation_rpm * 2.0f * std::numbers::pi_v<float> / 60.0f;

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("LOpenGL SDL3 - Rotating Triangle", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_05/rotating_triangle.vert.spv",
                    .fragment_shader        = "shaders/sdl3_05/rotating_triangle.frag.spv",
                    .vertex_uniform_buffers = 1,
                }
    );
    if (!pipeline_result) {
        std::println(stderr, "Pipeline creation failed: {}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    constexpr Uint32 vertex_data_size =
        static_cast<Uint32>(triangle_vertices.size() * sizeof(vertex_t));
    auto buffer_result = create_vertex_buffer(engine, triangle_vertices.data(), vertex_data_size);
    if (!buffer_result) {
        std::println(stderr, "Vertex buffer failed: {}", buffer_result.error());
        return 1;
    }
    gpu_buffer_t vertex_buffer = std::move(*buffer_result);

    float elapsed = 0.0f;

    while (poll_events()) {
        float dt = tick(engine);
        elapsed += dt;

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        float angle = elapsed * radians_per_second;

        // The pipeline never changes between frames -- only the angle pushed
        // into the command buffer changes. This is the key difference from
        // OpenGL's glUniform: the pipeline is immutable, data flows through it.
        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
                SDL_GPUBufferBinding binding = {vertex_buffer.get(), 0};
                SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
                push_vertex_uniform(cmd_buf, 0, angle);
                SDL_DrawGPUPrimitives(pass, triangle_vertices.size(), 1, 0, 0);
            }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
