#include "engine.hpp"
#include <SDL3/SDL_main.h>
#include <glm/gtc/matrix_transform.hpp>
#include <print>

constexpr std::string_view TITLE         = "Uniform buffer objects";
constexpr int              WINDOW_WIDTH  = 1024;
constexpr int              WINDOW_HEIGHT = 768;

struct scene_t {
    gpu_pipeline_t red_pipeline;
    gpu_pipeline_t green_pipeline;
    gpu_pipeline_t blue_pipeline;
    gpu_pipeline_t yellow_pipeline;
    gpu_geometry_t cube_geometry;
    camera_t       camera;
    float          m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    auto make_pipeline = [&](std::string_view frag) -> std::expected<gpu_pipeline_t, std::string> {
        return create_pipeline(
            engine, {
                        .vertex_shader          = "shaders/sdl3_29/ubo.vert.spv",
                        .fragment_shader        = frag,
                        .vertex_uniform_buffers = 3,
                        .vertex_buffer_descs    = pos_uv_buffer_descs,
                        .vertex_attributes      = pos_uv_vertex_attributes,
                        .enable_depth_test      = true,
                    }
        );
    };

    auto red = make_pipeline("shaders/sdl3_29/ubo_red.frag.spv");
    if (!red) return std::unexpected(red.error());
    auto green = make_pipeline("shaders/sdl3_29/ubo_green.frag.spv");
    if (!green) return std::unexpected(green.error());
    auto blue = make_pipeline("shaders/sdl3_29/ubo_blue.frag.spv");
    if (!blue) return std::unexpected(blue.error());
    auto yellow = make_pipeline("shaders/sdl3_29/ubo_yellow.frag.spv");
    if (!yellow) return std::unexpected(yellow.error());

    auto cube_geometry = create_vertex_geometry(
        engine, unit_cube.data(), static_cast<Uint32>(sizeof(unit_cube)),
        static_cast<Uint32>(unit_cube.size())
    );
    if (!cube_geometry) return std::unexpected(cube_geometry.error());

    return scene_t{
        .red_pipeline    = std::move(*red),
        .green_pipeline  = std::move(*green),
        .blue_pipeline   = std::move(*blue),
        .yellow_pipeline = std::move(*yellow),
        .cube_geometry   = std::move(*cube_geometry),
        .camera          = camera_t{engine.window, {0.0f, 0.0f, 3.0f}},
    };
}

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.update(in);
    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    auto const view = camera.rotation_view();
    auto const proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    // Push shared matrices once. Like a UBO, push constants persist across pipeline
    // binds — all four pipelines below read the same view and projection.
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);

    auto draw_cube = [&](gpu_pipeline_t const &pipeline, glm::vec3 position) {
        auto model = glm::translate(glm::mat4{1.0f}, position - camera.position);
        SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
        push_vertex_uniform(cmd, 0, model);
        SDL_GPUBufferBinding vb = {.buffer = cube_geometry.vertex_buffer.get(), .offset = 0};
        SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);
        SDL_DrawGPUPrimitives(pass, cube_geometry.vertex_count, 1, 0, 0);
    };

    draw_cube(red_pipeline, {-0.75f, 0.75f, 0.0f});
    draw_cube(green_pipeline, {0.75f, 0.75f, 0.0f});
    draw_cube(blue_pipeline, {-0.75f, -0.75f, 0.0f});
    draw_cube(yellow_pipeline, {0.75f, -0.75f, 0.0f});
}

int main(int argc, char *argv[]) {
    auto result = run_app(
        argc, argv, TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f},
        create_scene
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
