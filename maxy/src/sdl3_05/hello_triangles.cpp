#include <cstdint>
#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};
constexpr auto rhombus = make_equilateral_rhombus(0.5f);

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("LOpenGL SDL3 - Hello Triangles", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(
        engine, {
                    .vertex_shader = "shaders/sdl3_05/hello_triangle.vert.spv",
                    .fragment_shader = "shaders/sdl3_05/hello_triangle.frag.spv",
                }
    );
    if (!pipeline_result) {
        std::println(stderr, "Pipeline creation failed: {}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    constexpr Uint32 vertex_data_size =
        static_cast<Uint32>(rhombus.vertices.size() * sizeof(vertex_t));
    auto vbuf_result = create_vertex_buffer(engine, rhombus.vertices.data(), vertex_data_size);
    if (!vbuf_result) {
        std::println(stderr, "Vertex buffer failed: {}", vbuf_result.error());
        return 1;
    }
    gpu_buffer_t vertex_buffer = std::move(*vbuf_result);

    constexpr Uint32 index_data_size =
        static_cast<Uint32>(rhombus.indices.size() * sizeof(uint16_t));
    auto ibuf_result = create_index_buffer(engine, rhombus.indices.data(), index_data_size);
    if (!ibuf_result) {
        std::println(stderr, "Index buffer failed: {}", ibuf_result.error());
        return 1;
    }
    gpu_buffer_t index_buffer = std::move(*ibuf_result);

    while (poll_events()) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *, SDL_GPURenderPass *pass) {
                SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
                SDL_GPUBufferBinding vbinding = {vertex_buffer.get(), 0};
                SDL_BindGPUVertexBuffers(pass, 0, &vbinding, 1);
                SDL_GPUBufferBinding ibinding = {index_buffer.get(), 0};
                SDL_BindGPUIndexBuffer(pass, &ibinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
                SDL_DrawGPUIndexedPrimitives(pass, rhombus.indices.size(), 1, 0, 0, 0);
            }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
