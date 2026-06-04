#include <array>
#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"

struct vertex_t {
    float x, y, z;
};

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

constexpr std::array<vertex_t, 3> triangle_vertices = {{
    {-0.5f, -0.5f, 0.0f},
    {0.5f, -0.5f, 0.0f},
    {0.0f, 0.5f, 0.0f},
}};

namespace {

std::expected<gpu_pipeline_t, std::string> create_pipeline(engine_t const &engine) {
    auto vert = load_shader(engine, "shaders/hello_triangle.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vert) return std::unexpected(vert.error());

    auto frag =
        load_shader(engine, "shaders/hello_triangle.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);
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
    auto engine_result = create_engine("LOpenGL SDL3 - Hello Triangle", 800, 600, config);
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
        static_cast<Uint32>(triangle_vertices.size() * sizeof(vertex_t));
    auto buffer_result = create_vertex_buffer(engine, triangle_vertices.data(), vertex_data_size);
    if (!buffer_result) {
        std::println(stderr, "Vertex buffer failed: {}", buffer_result.error());
        return 1;
    }
    gpu_buffer_t vertex_buffer = std::move(*buffer_result);

    while (poll_events()) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *, SDL_GPURenderPass *pass) {
                SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
                SDL_GPUBufferBinding binding = {vertex_buffer.get(), 0};
                SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
                SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
            }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
