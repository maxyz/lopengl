#include <cmath>
#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"

// ex_1 vs strobe: the light cube now shows the actual strobe color.
// The light pipeline gains fragment_uniform_buffers=1 and light_strobe.frag,
// which reads light.ambient + light.diffuse from the same push as the cubes.
// In OpenGL set_light() works identically on both pipelines via named uniforms;
// SDL3 requires the explicit declaration and an extra push_fragment_uniform call.

constexpr int        WINDOW_WIDTH     = 800;
constexpr int        WINDOW_HEIGHT    = 600;
constexpr SDL_FColor BACKGROUND_COLOR = {0.1f, 0.1f, 0.1f, 1.0f};
constexpr float      LIGHT_SPEED      = 2.5f;

struct material_uniforms_t {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular_shininess; // .rgb = specular, .w = shininess
};

struct light_uniforms_t {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 position; // camera-relative world space; updated each frame
};

constexpr material_uniforms_t pearl = {
    .ambient            = {0.25f, 0.20725f, 0.20725f, 0.0f},
    .diffuse            = {1.0f, 0.829f, 0.829f, 0.0f},
    .specular_shininess = {0.296648f, 0.296648f, 0.296648f, 11.264f},
};

constexpr light_uniforms_t initial_light = {
    .ambient  = {0.2f, 0.2f, 0.2f, 0.0f},
    .diffuse  = {0.5f, 0.5f, 0.5f, 0.0f},
    .specular = {1.0f, 1.0f, 1.0f, 0.0f},
    .position = {2.0f, 1.0f, -2.0f, 0.0f},
};

light_uniforms_t strobe_light(light_uniforms_t base, float time) {
    glm::vec3 color = {std::sin(time) * 2.0f, std::sin(time) * 0.7f, std::sin(time) * 1.3f};
    base.diffuse    = glm::vec4(color * 0.5f, 0.0f);
    base.ambient    = glm::vec4(glm::vec3(base.diffuse) * 0.2f, 0.0f);
    return base;
}

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

    camera_t         camera{{0.0f, 0.0f, 3.0f}};
    glm::vec3        light_position = {2.0f, 1.0f, -2.0f};
    light_uniforms_t light          = initial_light;
    float            m_time         = 0.0f;
    float            m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

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

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::vec3 light_rot  = light_position + glm::vec3(std::sin(m_time), 0.0f, std::cos(m_time));
    glm::mat4 view       = glm::mat4(glm::mat3(camera.view_matrix()));
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    light_uniforms_t frame_light = strobe_light(light, m_time);
    frame_light.position         = glm::vec4(light_rot - camera.position, 0.0f);

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, pearl);
    push_fragment_uniform(cmd, 1, frame_light);

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
    // Push strobe color to the light cube pipeline. Specular and position in
    // frame_light are ignored by light_strobe.frag -- it only reads ambient+diffuse.
    push_fragment_uniform(cmd, 0, frame_light);
    draw(light_pipeline, geometry, gpu_material_t{}, pass);
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_14/cube.vert.spv",
                    .fragment_shader          = "shaders/sdl3_14/cube.frag.spv",
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

    auto light_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_14/light.vert.spv",
                    .fragment_shader          = "shaders/sdl3_14/light_strobe.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = light_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!light_pipe) return std::unexpected(light_pipe.error());
    scene.light_pipeline = std::move(*light_pipe);

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
    auto result = run_app(
        argc, argv, "SDL3 14 - Ex 1: Colored Light Cube", WINDOW_WIDTH, WINDOW_HEIGHT,
        BACKGROUND_COLOR, create_scene
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
