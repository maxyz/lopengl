#include <print>
#include <string>

#include <SDL3/SDL.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

constexpr SDL_FColor background_color = {0.2f, 0.3f, 0.3f, 1.0f};

// UVs zoomed into the centre (0.49..0.51) -- only the central ~2% of the
// texture is sampled, magnifying individual pixels across the whole quad.
constexpr std::array<textured_vertex_t, 4> quad_vertices = {{
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.51f, 0.51f}},   // top right
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.49f, 0.51f}},  // top left
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.51f, 0.49f}},  // bottom right
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.49f, 0.49f}}, // bottom left
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
        create_engine("SDL3 ex 7.8.3 - Pixel Zoom", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!engine_result) {
        std::println(stderr, "Engine init failed: {}", engine_result.error());
        return 1;
    }
    engine_t &engine = *engine_result;

    auto mesh_result = create_textured_mesh(
        engine, {
                    .vertex_shader       = "shaders/sdl3_07/texture.vert.spv",
                    .fragment_shader     = "shaders/sdl3_07/ex_7_8_2.frag.spv",
                    .vertex_buffer_descs = buffer_descs,
                    .vertex_attributes   = vertex_attributes,
                    .vertices            = quad_vertices.data(),
                    .vertex_data_size =
                        static_cast<Uint32>(quad_vertices.size() * sizeof(textured_vertex_t)),
                    .indices = quad_indices,
                    .texture_paths =
                        {
                            std::string(ASSETS_PATH) + "textures/container.jpg",
                            std::string(ASSETS_PATH) + "textures/awesomeface.png",
                        },
                    .address_modes =
                        {
                            SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
                            SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                        },
                    // NEAREST filtering: each pixel snaps to the nearest texel, making
                    // the individual pixels clearly visible when magnified.
                    .filter_modes = {
                        SDL_GPU_FILTER_NEAREST,
                        SDL_GPU_FILTER_NEAREST,
                    },
                }
    );
    if (!mesh_result) {
        std::println(stderr, "Mesh creation failed: {}", mesh_result.error());
        return 1;
    }
    textured_mesh_t mesh = std::move(*mesh_result);

    while (poll_events()) {
        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        auto frame = render_frame(
            engine, background_color,
            [&](SDL_GPUCommandBuffer *, SDL_GPURenderPass *pass) { draw(mesh, pass); }
        );
        if (!frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }
    }

    return 0;
}
