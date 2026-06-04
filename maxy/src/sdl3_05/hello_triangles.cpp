#include <cstdint>
#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};
constexpr auto rhombus = make_equilateral_rhombus(0.5f);

namespace {

std::expected<gpu_pipeline_t, std::string> create_pipeline(engine_t const &engine) {
    auto vert =
        load_shader(engine, "shaders/sdl3_05/hello_triangle.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vert) return std::unexpected(vert.error());

    auto frag = load_shader(
        engine, "shaders/sdl3_05/hello_triangle.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT
    );
    if (!frag) return std::unexpected(frag.error());

    SDL_GPUVertexBufferDescription buffer_desc = {};
    buffer_desc.slot = 0;
    buffer_desc.pitch = sizeof(vertex_t);
    buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attribute = {};
    attribute.location = 0;
    attribute.buffer_slot = 0;
    attribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribute.offset = 0;

    SDL_GPUColorTargetDescription color_target = {};
    color_target.format = SDL_GetGPUSwapchainTextureFormat(engine.gpu_device, engine.window);

    SDL_GPUGraphicsPipelineCreateInfo info = {};
    info.vertex_shader = vert->get();
    info.fragment_shader = frag->get();
    info.vertex_input_state.vertex_buffer_descriptions = &buffer_desc;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = &attribute;
    info.vertex_input_state.num_vertex_attributes = 1;
    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.target_info.color_target_descriptions = &color_target;
    info.target_info.num_color_targets = 1;

    gpu_pipeline_t pipeline{
        engine.gpu_device, SDL_CreateGPUGraphicsPipeline(engine.gpu_device, &info)
    };
    if (!pipeline) return sdl_error("SDL_CreateGPUGraphicsPipeline failed");
    return pipeline;
}

} // namespace

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("LOpenGL SDL3 - Hello Triangles", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(engine);
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
