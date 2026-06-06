#include <cmath>
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
constexpr float      LIGHT_SPEED      = 2.5f;

struct lighting_t {
    glm::vec4 object_color;
    glm::vec4 light_color;
    glm::vec4 light_strengths; // x=ambient, y=diffuse, z=specular, w=shininess
};

struct positions_t {
    glm::vec4 light_pos; // camera-relative; view_pos absent: camera is at origin
};

constexpr SDL_GPUVertexAttribute light_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, position))},
};

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t light_pipeline;
    gpu_geometry_t geometry;
    gpu_material_t material;

    camera_t   camera{{0.0f, 0.0f, 3.0f}};
    glm::vec3  light_position = {2.0f, 1.0f, -2.0f};
    lighting_t lighting       = {
        .object_color    = {1.0f, 0.5f, 0.31f, 0.0f},
        .light_color     = {1.0f, 1.0f, 1.0f, 0.0f},
        .light_strengths = {0.1f, 1.0f, 0.5f, 32.0f},
    };
    float m_time         = 0.0f;
    float m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_time += in.dt;
    m_aspect_ratio = in.aspect_ratio;

    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.process_keys(in.keys, in.dt);
    camera.process_mouse(in.dx, in.dy);
    camera.process_scroll(in.scroll);

    float step = LIGHT_SPEED * in.dt;
    if (in.keys[SDL_SCANCODE_I]) light_position.z -= step;
    if (in.keys[SDL_SCANCODE_K]) light_position.z += step;
    if (in.keys[SDL_SCANCODE_J]) light_position.x -= step;
    if (in.keys[SDL_SCANCODE_L]) light_position.x += step;
    if (in.keys[SDL_SCANCODE_Y]) light_position.y += step;
    if (in.keys[SDL_SCANCODE_H]) light_position.y -= step;

    bool  shift = in.keys[SDL_SCANCODE_LSHIFT] || in.keys[SDL_SCANCODE_RSHIFT];
    auto &ls    = lighting.light_strengths;
    if (in.keys[SDL_SCANCODE_Z]) {
        ls.x += shift ? 0.01f : -0.01f;
        std::println("Ambient: {:.3f}", ls.x);
    }
    if (in.keys[SDL_SCANCODE_X]) {
        ls.y += shift ? 0.01f : -0.01f;
        std::println("Diffuse: {:.3f}", ls.y);
    }
    if (in.keys[SDL_SCANCODE_C]) {
        ls.z += shift ? 0.01f : -0.01f;
        std::println("Specular: {:.3f}", ls.z);
    }
    if (in.keys[SDL_SCANCODE_V]) {
        ls.w = shift ? ls.w * 2.0f : ls.w / 2.0f;
        std::println("Shininess: {:.3f}", ls.w);
    }
    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::vec3 light_rot  = light_position + glm::vec3(std::sin(m_time), 0.0f, std::cos(m_time));
    glm::mat4 view       = glm::mat4(glm::mat3(camera.view_matrix()));
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    positions_t positions{.light_pos = glm::vec4(light_rot - camera.position, 0.0f)};

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, lighting);
    push_fragment_uniform(cmd, 1, positions);

    for (Uint32 i = 0; i < example_cube_positions.size(); ++i) {
        float     angle  = m_time * static_cast<float>(i % 3) * 25.0f;
        glm::vec3 camrel = example_cube_positions[i] - camera.position;
        glm::mat4 model  = glm::rotate(
            glm::translate(glm::mat4(1.0f), camrel), glm::radians(angle),
            glm::vec3(1.0f, 0.3f, 0.5f)
        );
        push_vertex_uniform(cmd, 0, model);
        draw(cube_pipeline, geometry, material, pass);
    }

    glm::mat4 light_model =
        glm::scale(glm::translate(glm::mat4(1.0f), light_rot - camera.position), glm::vec3(0.2f));
    push_vertex_uniform(cmd, 0, light_model);
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    draw(light_pipeline, geometry, gpu_material_t{}, pass);
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_13/cube_camrel.vert.spv",
                    .fragment_shader          = "shaders/sdl3_13/cube_camrel.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 2,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!cube) return std::unexpected(cube.error());
    scene.cube_pipeline = std::move(*cube);

    auto light = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_13/light.vert.spv",
                    .fragment_shader        = "shaders/sdl3_13/light.frag.spv",
                    .vertex_uniform_buffers = 3,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = light_vertex_attributes,
                    .enable_depth_test      = true,
                }
    );
    if (!light) return std::unexpected(light.error());
    scene.light_pipeline = std::move(*light);

    auto geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!geom) return std::unexpected(geom.error());
    scene.geometry = std::move(*geom);

    auto mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/container.jpg",
                     std::string(ASSETS_PATH) + "textures/awesomeface.png",
                 }}
    );
    if (!mat) return std::unexpected(mat.error());
    scene.material = std::move(*mat);

    return scene;
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 13 - Camrel Loop", WINDOW_WIDTH, WINDOW_HEIGHT, parse_engine_args(argc, argv)
    );
    if (!engine) {
        std::println(stderr, "{}", engine.error());
        return 1;
    }

    auto scene = create_scene(*engine);
    if (!scene) {
        std::println(stderr, "{}", scene.error());
        return 1;
    }

    auto result = run_loop(
        *engine, BACKGROUND_COLOR, [&](input_t const &in) { return scene->update(in); },
        [&](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) { scene->render(cmd, pass); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
