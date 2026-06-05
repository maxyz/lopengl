#include <algorithm>
#include <print>
#include <string>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int   WINDOW_WIDTH  = 800;
constexpr int   WINDOW_HEIGHT = 600;
constexpr float LEVEL_STEP    = 0.5f;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

constexpr std::array<textured_vertex_t, 4> quad_vertices = {{
    {{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
}};

constexpr std::array<uint16_t, 6> quad_indices = {{0, 2, 3, 0, 1, 3}};

constexpr SDL_GPUVertexBufferDescription buffer_descs[] = {{
    .slot       = 0,
    .pitch      = sizeof(textured_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
}};

constexpr SDL_GPUVertexAttribute vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(textured_vertex_t, position))},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(textured_vertex_t, color))},
    {.location    = 2,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
     .offset      = static_cast<Uint32>(offsetof(textured_vertex_t, uv))},
};

int main(int argc, char *argv[]) {
    auto config = parse_engine_args(argc, argv);
    auto engine_result =
        create_engine("SDL3 ex 7.8.4 - Mix Level", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto pipeline_result = create_pipeline(engine, {
        .vertex_shader            = "shaders/sdl3_07/texture.vert.spv",
        .fragment_shader          = "shaders/sdl3_07/ex_7_8_4.frag.spv",
        .fragment_uniform_buffers = 1,
        .fragment_samplers        = 2,
        .vertex_buffer_descs      = buffer_descs,
        .vertex_attributes        = vertex_attributes,
    });
    if (!pipeline_result) {
        std::println(stderr, "Pipeline failed: {}", pipeline_result.error());
        return 1;
    }
    gpu_pipeline_t pipeline = std::move(*pipeline_result);

    auto geometry_result = create_geometry(
        engine, quad_vertices.data(),
        static_cast<Uint32>(quad_vertices.size() * sizeof(textured_vertex_t)), quad_indices);
    if (!geometry_result) {
        std::println(stderr, "Geometry failed: {}", geometry_result.error());
        return 1;
    }
    gpu_geometry_t geometry = std::move(*geometry_result);

    auto material_result = create_material(engine, {
        .texture_paths = {
            std::string(ASSETS_PATH) + "textures/container.jpg",
            std::string(ASSETS_PATH) + "textures/awesomeface.png",
        },
    });
    if (!material_result) {
        std::println(stderr, "Material failed: {}", material_result.error());
        return 1;
    }
    gpu_material_t material = std::move(*material_result);

    float level = 0.2f;

    while (poll_events()) {
        float dt = tick(engine);

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;
        if (keys[SDL_SCANCODE_UP])   level = std::clamp(level + LEVEL_STEP * dt, 0.0f, 1.0f);
        if (keys[SDL_SCANCODE_DOWN]) level = std::clamp(level - LEVEL_STEP * dt, 0.0f, 1.0f);

        auto frame = render_frame(
            engine, background_color, [&](SDL_GPUCommandBuffer *cmd_buf, SDL_GPURenderPass *pass) {
                push_fragment_uniform(cmd_buf, 0, level);
                draw(pipeline, geometry, material, pass);
            }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
