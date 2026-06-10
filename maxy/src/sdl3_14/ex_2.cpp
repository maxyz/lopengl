#include <array>
#include <cmath>
#include <print>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int        WINDOW_WIDTH     = 800;
constexpr int        WINDOW_HEIGHT    = 600;
constexpr SDL_FColor BACKGROUND_COLOR = {0.1f, 0.1f, 0.1f, 1.0f};
constexpr float      LIGHT_SPEED      = 2.5f;

struct material_uniforms_t {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular_shininess; // .rgb = specular, .w = shininess
};

// use_texture is pushed as a per-draw flag so the shader can bypass sampling.
// Drivers treat a uniform-driven branch as a select (no per-fragment divergence).
struct flags_uniforms_t {
    Uint32 use_texture;
    Uint32 pad0, pad1, pad2; // std140: minimum block size is 16 bytes
};

// Source: http://devernay.free.fr/cours/opengl/materials.html
// Shininess is scaled from [0,1] coefficient to [0,128] (max_shininess = 128).
constexpr std::array<material_uniforms_t, 23> materials = {{
    // emerald
    {{0.0215f, 0.1745f, 0.0215f, 0.0f},
     {0.07568f, 0.61424f, 0.07568f, 0.0f},
     {0.633f, 0.727811f, 0.633f, 76.8f}},
    // jade
    {{0.135f, 0.2225f, 0.1575f, 0.0f},
     {0.54f, 0.89f, 0.63f, 0.0f},
     {0.316228f, 0.316228f, 0.316228f, 12.8f}},
    // obsidian
    {{0.05375f, 0.05f, 0.06625f, 0.0f},
     {0.18275f, 0.17f, 0.22525f, 0.0f},
     {0.332741f, 0.328634f, 0.346435f, 38.4f}},
    // pearl
    {{0.25f, 0.20725f, 0.20725f, 0.0f},
     {1.0f, 0.829f, 0.829f, 0.0f},
     {0.296648f, 0.296648f, 0.296648f, 11.264f}},
    // ruby
    {{0.1745f, 0.01175f, 0.01175f, 0.0f},
     {0.61424f, 0.04136f, 0.04136f, 0.0f},
     {0.727811f, 0.626959f, 0.626959f, 76.8f}},
    // turquoise
    {{0.1f, 0.18725f, 0.1745f, 0.0f},
     {0.396f, 0.74151f, 0.69102f, 0.0f},
     {0.297254f, 0.30829f, 0.306678f, 12.8f}},
    // brass
    {{0.329412f, 0.223529f, 0.027451f, 0.0f},
     {0.780392f, 0.568627f, 0.113725f, 0.0f},
     {0.992157f, 0.941176f, 0.807843f, 27.897f}},
    // bronze
    {{0.2125f, 0.1275f, 0.054f, 0.0f},
     {0.714f, 0.4284f, 0.18144f, 0.0f},
     {0.393548f, 0.271906f, 0.166721f, 25.6f}},
    // chrome
    {{0.25f, 0.25f, 0.25f, 0.0f},
     {0.4f, 0.4f, 0.4f, 0.0f},
     {0.774597f, 0.774597f, 0.774597f, 76.8f}},
    // copper
    {{0.19125f, 0.0735f, 0.0225f, 0.0f},
     {0.7038f, 0.27048f, 0.0828f, 0.0f},
     {0.256777f, 0.137622f, 0.086014f, 12.8f}},
    // gold
    {{0.24725f, 0.1995f, 0.0745f, 0.0f},
     {0.75164f, 0.60648f, 0.22648f, 0.0f},
     {0.628281f, 0.555802f, 0.366065f, 51.2f}},
    // silver
    {{0.19225f, 0.19225f, 0.19225f, 0.0f},
     {0.50754f, 0.50754f, 0.50754f, 0.0f},
     {0.508273f, 0.508273f, 0.508273f, 51.2f}},
    // black_plastic
    {{0.0f, 0.0f, 0.0f, 0.0f}, {0.01f, 0.01f, 0.01f, 0.0f}, {0.50f, 0.50f, 0.50f, 32.0f}},
    // cyan_plastic
    {{0.0f, 0.1f, 0.06f, 0.0f},
     {0.0f, 0.50980392f, 0.50980392f, 0.0f},
     {0.50196078f, 0.50196078f, 0.50196078f, 32.0f}},
    // green_plastic
    {{0.0f, 0.0f, 0.0f, 0.0f}, {0.1f, 0.35f, 0.1f, 0.0f}, {0.45f, 0.55f, 0.45f, 32.0f}},
    // red_plastic
    {{0.0f, 0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f, 0.0f}, {0.7f, 0.6f, 0.6f, 32.0f}},
    // white_plastic
    {{0.0f, 0.0f, 0.0f, 0.0f}, {0.55f, 0.55f, 0.55f, 0.0f}, {0.70f, 0.70f, 0.70f, 32.0f}},
    // yellow_plastic
    {{0.0f, 0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 0.0f}, {0.60f, 0.60f, 0.50f, 32.0f}},
    // black_rubber
    {{0.02f, 0.02f, 0.02f, 0.0f}, {0.01f, 0.01f, 0.01f, 0.0f}, {0.4f, 0.4f, 0.4f, 10.0f}},
    // green_rubber
    {{0.0f, 0.05f, 0.0f, 0.0f}, {0.4f, 0.5f, 0.4f, 0.0f}, {0.04f, 0.7f, 0.04f, 10.0f}},
    // red_rubber
    {{0.05f, 0.0f, 0.0f, 0.0f}, {0.5f, 0.4f, 0.4f, 0.0f}, {0.7f, 0.04f, 0.04f, 10.0f}},
    // white_rubber
    {{0.05f, 0.05f, 0.05f, 0.0f}, {0.5f, 0.5f, 0.5f, 0.0f}, {0.7f, 0.7f, 0.7f, 10.0f}},
    // yellow_rubber
    {{0.05f, 0.05f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.4f, 0.0f}, {0.7f, 0.7f, 0.04f, 10.0f}},
}};

constexpr light_uniforms_t initial_light = {
    .ambient  = {0.2f, 0.2f, 0.2f, 0.0f},
    .diffuse  = {0.5f, 0.5f, 0.5f, 0.0f},
    .specular = {1.0f, 1.0f, 1.0f, 0.0f},
    .position = {2.0f, 1.0f, -2.0f, 0.0f},
};

// cos on blue is 90 deg out of phase with red/green, giving mild hue cycling
// rather than pure brightness strobe (all-sin would only vary intensity).
light_uniforms_t strobe_light(light_uniforms_t base, float time) {
    glm::vec3 color = {std::sin(time) * 2.0f, std::sin(time) * 0.7f, std::cos(time) * 1.3f};
    base.diffuse    = glm::vec4(color * 0.5f, 0.0f);
    base.ambient    = glm::vec4(glm::vec3(base.diffuse) * 0.2f, 0.0f);
    return base;
}

struct scene_t {
    gpu_pipeline_t cube_pipeline;
    gpu_pipeline_t light_pipeline;
    gpu_geometry_t geometry;
    gpu_material_t material;

    camera_t         camera{{0.0f, 0.0f, 3.0f}};
    glm::vec3        light_position = {2.0f, 1.0f, -2.0f};
    light_uniforms_t light          = initial_light;
    size_t           m_current_mat  = 3; // pearl as default (index 3)
    bool             m_textures_on  = true;
    key_edge_t       m_m_edge;
    key_edge_t       m_t_edge;
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

    if (m_m_edge(in.keys[SDL_SCANCODE_M])) {
        bool shift = in.keys[SDL_SCANCODE_LSHIFT] || in.keys[SDL_SCANCODE_RSHIFT];
        if (shift) {
            m_current_mat = (m_current_mat - 1 + materials.size()) % materials.size();
        } else {
            m_current_mat = (m_current_mat + 1) % materials.size();
        }
    }

    if (m_t_edge(in.keys[SDL_SCANCODE_T])) m_textures_on = !m_textures_on;

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

    flags_uniforms_t flags = {.use_texture = m_textures_on ? 1u : 0u};

    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);
    push_fragment_uniform(cmd, 0, materials[m_current_mat]);
    push_fragment_uniform(cmd, 1, frame_light);
    push_fragment_uniform(cmd, 2, flags);

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
    push_fragment_uniform(cmd, 0, frame_light);
    draw(light_pipeline, geometry, gpu_material_t{}, pass);
}

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;

    auto cube = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_14/cube.vert.spv",
                    .fragment_shader          = "shaders/sdl3_14/cube_matsel.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 3,
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
        argc, argv, "SDL3 14 - Ex 2: Material Selector", WINDOW_WIDTH, WINDOW_HEIGHT,
        BACKGROUND_COLOR, create_scene
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
