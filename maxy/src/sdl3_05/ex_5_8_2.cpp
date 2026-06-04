#include <numbers>
#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

constexpr float circumradius = 0.3f;
constexpr float side_length  = circumradius * std::numbers::sqrt3_v<float>;

// One centered triangle drawn twice with different translation offsets --
// the SDL3_GPU equivalent of two VAOs/VBOs: same geometry, different positions.
constexpr auto triangle = make_equilateral_triangle(circumradius);

struct vec2_t {
    float x, y;
};

constexpr vec2_t left_offset  = {-side_length / 2.0f, 0.0f};
constexpr vec2_t right_offset = {side_length / 2.0f, 0.0f};

int main(int argc, char *argv[]) {
    auto config        = parse_engine_args(argc, argv);
    auto engine_result = create_engine("SDL3 ex 5.8.2", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_05/ex_5_8_2.vert.spv",
                    .fragment_shader        = "shaders/sdl3_05/hello_triangle.frag.spv",
                    .vertex_uniform_buffers = 1,
                }
    );
    if (!pipeline_result) {
        std::println(stderr, "Pipeline creation failed: {}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    constexpr Uint32 vertex_data_size = static_cast<Uint32>(triangle.size() * sizeof(vertex_t));
    auto buffer_result = create_vertex_buffer(engine, triangle.data(), vertex_data_size);
    if (!buffer_result) {
        std::println(stderr, "Vertex buffer failed: {}", buffer_result.error());
        return 1;
    }
    gpu_buffer_t vertex_buffer = std::move(*buffer_result);

    while (poll_events()) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
                SDL_GPUBufferBinding binding = {vertex_buffer.get(), 0};
                SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);

                push_vertex_uniform(cmd_buf, 0, left_offset);
                SDL_DrawGPUPrimitives(pass, triangle.size(), 1, 0, 0);

                push_vertex_uniform(cmd_buf, 0, right_offset);
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
