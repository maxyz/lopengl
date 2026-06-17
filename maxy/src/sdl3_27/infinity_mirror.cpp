// infinity_mirror.cpp — two large flat mirrors facing each other with a box
// between them, producing a recursive "infinity mirror" tunnel.
//
// Each mirror is a flat quad that reflects a cubemap rendered from the mirror's
// centre. The cubemap captures the box and the OTHER mirror; the other mirror
// is itself drawn reflecting its own cubemap (reused from the previous frame),
// so each frame adds one more bounce and the reflections converge into the
// receding tunnel -- the same frame-reuse trick as the cube example, applied to
// flat mirrors. A point cubemap is only an approximation of a flat mirror, so
// the reflections are not parallax-exact, but they recurse and track the camera.
//
// The box spins slowly, so the lag is visible: deeper reflections show
// progressively older orientations.

#include <array>
#include <print>
#include <span>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int      WINDOW_WIDTH  = 1024;
constexpr int      WINDOW_HEIGHT = 768;
constexpr uint32_t CUBEMAP_SIZE  = 512;

constexpr int   NUM_MIRRORS    = 2;
constexpr float MIRROR_GAP     = 4.0f; // mirrors sit at z = -GAP and z = +GAP
constexpr float MIRROR_WIDTH   = 7.0f;
constexpr float MIRROR_HEIGHT  = 5.0f;
constexpr float BOX_SIZE       = 0.9f;
constexpr float BOX_SPIN_SPEED = 0.6f; // radians/second
// The mirrors are toed in by opposite amounts so they are NOT exactly
// anti-parallel. Perfectly parallel mirrors stack every reflection directly
// behind the previous one (all hidden by the front copy); a slight tilt makes
// each successive virtual image step sideways, so the tunnel is distinguishable.
constexpr float MIRROR_TILT_DEGREES = 10.0f;

static constexpr std::array<glm::vec3, NUM_MIRRORS> MIRROR_CENTERS = {{
    {0.0f, 0.0f, -MIRROR_GAP},
    {0.0f, 0.0f, MIRROR_GAP},
}};

static const glm::vec3 LIGHT_DIRECTION = glm::normalize(glm::vec3{0.3f, -0.6f, -0.5f});

// Cubemap face render orientations (see sdl3_27 inter_reflection for the
// derivation). Layer 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z.
static constexpr std::array<glm::vec3, 6> FACE_TARGETS = {{
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1},
}};
static constexpr std::array<glm::vec3, 6> FACE_UPS = {{
    {0, 1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1}, {0, 1, 0}, {0, 1, 0},
}};

struct light_block_t {
    glm::vec4 light_direction; // world space
};

namespace {

std::expected<gpu_texture_t, std::string> create_dynamic_cubemap(engine_t const &engine) {
    SDL_GPUTextureFormat fmt = SDL_GetGPUSwapchainTextureFormat(engine.gpu_device, engine.window);

    SDL_GPUTextureCreateInfo info = {};
    info.type                     = SDL_GPU_TEXTURETYPE_CUBE;
    info.format                   = fmt;
    info.usage                = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    info.width                = CUBEMAP_SIZE;
    info.height               = CUBEMAP_SIZE;
    info.layer_count_or_depth = 6;
    info.num_levels           = 1;
    gpu_texture_t tex{engine.gpu_device, SDL_CreateGPUTexture(engine.gpu_device, &info)};
    if (!tex) return sdl_error("SDL_CreateGPUTexture (dynamic cubemap) failed");
    return tex;
}

// Mirror A faces +Z, mirror B faces -Z (180 deg turn). Each is additionally
// toed in by an opposite tilt about Y so the two normals are not anti-parallel.
// The flat reflection uses the quad's normal, so this tilt is what offsets the
// successive virtual images. (The cubemaps are world-axis-aligned and capture
// every direction, so tilting the quad needs no change to cubemap rendering.)
glm::mat4 mirror_model(int index, glm::vec3 const &cam_pos) {
    float const facing = (index == 1) ? 180.0f : 0.0f;
    float const tilt   = (index == 0) ? MIRROR_TILT_DEGREES : -MIRROR_TILT_DEGREES;
    glm::mat4   model  = glm::translate(glm::mat4{1.0f}, MIRROR_CENTERS[index] - cam_pos);
    model = glm::rotate(model, glm::radians(facing + tilt), glm::vec3{0.0f, 1.0f, 0.0f});
    return glm::scale(model, glm::vec3{MIRROR_WIDTH, MIRROR_HEIGHT, 1.0f});
}

} // namespace

struct scene_t {
    gpu_pipeline_t box_pipeline;
    gpu_pipeline_t mirror_pipeline;

    gpu_geometry_t box_geometry;
    gpu_geometry_t mirror_geometry;
    gpu_material_t box_material;

    // One persistent cubemap per mirror, reused across frames to build up the
    // recursive tunnel one bounce at a time.
    gpu_texture_t mirror_cubemaps[NUM_MIRRORS];
    gpu_texture_t cubemap_depth; // shared by all face passes (cycled per pass)
    gpu_sampler_t cubemap_sampler;

    tracked_depth_t        main_depth;
    tracked_color_target_t unused_color_target; // required by the multi-pass run_loop API

    camera_t camera;
    float    m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float    m_box_angle    = 0.0f;
    int      m_frame_count  = 0;

    bool update(input_t const &in);

    glm::mat4 box_model(glm::vec3 const &cam_pos) const {
        glm::mat4 model = glm::translate(glm::mat4{1.0f}, -cam_pos);
        model           = glm::rotate(model, m_box_angle, glm::vec3{0.0f, 1.0f, 0.0f});
        return glm::scale(model, glm::vec3{BOX_SIZE});
    }

    void render_box(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
        glm::mat4 const &view, glm::mat4 const &proj
    );
    void render_mirror(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
        glm::mat4 const &view, glm::mat4 const &proj, int mirror_index, SDL_GPUTexture *cubemap
    );
    void render_cubemap_face(SDL_GPUCommandBuffer *cmd, int mirror_index, int face);
    void render_all_cubemap_faces(SDL_GPUCommandBuffer *cmd);
    void render_main(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window, {1.0f, 0.6f, 3.0f}, -90.0f, -6.0f);

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    auto box_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_27/box.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 1,
                    .fragment_samplers        = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                    .cull_mode                = SDL_GPU_CULLMODE_NONE,
                }
    );
    if (!box_pipe) return std::unexpected(box_pipe.error());
    scene.box_pipeline = std::move(*box_pipe);

    // Cubemap faces are rendered with a flipped-X projection (winding inverted),
    // so culling is off.
    auto mirror_pipe = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader        = "shaders/sdl3_27/mirror.frag.spv",
                    .vertex_uniform_buffers = 4,
                    .fragment_samplers      = 1,
                    .vertex_buffer_descs    = pos_normal_uv_buffer_descs,
                    .vertex_attributes      = pos_normal_uv_vertex_attributes,
                    .enable_depth_test      = true,
                    .cull_mode              = SDL_GPU_CULLMODE_NONE,
                }
    );
    if (!mirror_pipe) return std::unexpected(mirror_pipe.error());
    scene.mirror_pipeline = std::move(*mirror_pipe);

    auto box_geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!box_geom) return std::unexpected(box_geom.error());
    scene.box_geometry = std::move(*box_geom);

    auto mirror_geom = create_vertex_geometry(
        engine, vertical_quad_vertices.data(),
        static_cast<Uint32>(vertical_quad_vertices.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(vertical_quad_vertices.size())
    );
    if (!mirror_geom) return std::unexpected(mirror_geom.error());
    scene.mirror_geometry = std::move(*mirror_geom);

    auto box_mat = create_material(
        engine, {.texture_paths = {std::string(ASSETS_PATH) + "textures/container2.png"}}
    );
    if (!box_mat) return std::unexpected(box_mat.error());
    scene.box_material = std::move(*box_mat);

    for (int i = 0; i < NUM_MIRRORS; ++i) {
        auto cm = create_dynamic_cubemap(engine);
        if (!cm) return std::unexpected(cm.error());
        scene.mirror_cubemaps[i] = std::move(*cm);
    }

    auto depth = create_depth_texture(engine, CUBEMAP_SIZE, CUBEMAP_SIZE);
    if (!depth) return std::unexpected(depth.error());
    scene.cubemap_depth = std::move(*depth);

    auto sampler = create_sampler(engine, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
    if (!sampler) return std::unexpected(sampler.error());
    scene.cubemap_sampler = std::move(*sampler);

    auto main_depth = create_tracked_depth(engine);
    if (!main_depth) return std::unexpected(main_depth.error());
    scene.main_depth = std::move(*main_depth);

    auto unused = create_tracked_color_target(engine);
    if (!unused) return std::unexpected(unused.error());
    scene.unused_color_target = std::move(*unused);

    return scene;
}

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.update(in);
    m_box_angle += BOX_SPIN_SPEED * in.dt;

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Infinity Mirror", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::End();

    ++m_frame_count;
    return true;
}

void scene_t::render_box(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
    glm::mat4 const &view, glm::mat4 const &proj
) {
    SDL_BindGPUGraphicsPipeline(pass, box_pipeline.get());
    push_vertex_uniform(cmd, 0, box_model(cam_pos));
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_vertex_uniform(cmd, 3, 1.0f); // normal_flip

    light_block_t light = {glm::vec4(LIGHT_DIRECTION, 0.0f)};
    push_fragment_uniform(cmd, 0, light);
    draw(box_geometry, box_material, pass);
}

void scene_t::render_mirror(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
    glm::mat4 const &view, glm::mat4 const &proj, int mirror_index, SDL_GPUTexture *cubemap
) {
    SDL_BindGPUGraphicsPipeline(pass, mirror_pipeline.get());
    push_vertex_uniform(cmd, 0, mirror_model(mirror_index, cam_pos));
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);
    push_vertex_uniform(cmd, 3, 1.0f);

    SDL_GPUTextureSamplerBinding binding = {cubemap, cubemap_sampler.get()};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);
    draw(mirror_geometry, gpu_material_t{}, pass);
}

void scene_t::render_cubemap_face(SDL_GPUCommandBuffer *cmd, int mirror_index, int face) {
    SDL_GPUColorTargetInfo color_info = {};
    color_info.texture                = mirror_cubemaps[mirror_index].get();
    color_info.layer_or_depth_plane   = static_cast<Uint32>(face);
    color_info.load_op                = SDL_GPU_LOADOP_CLEAR;
    color_info.store_op               = SDL_GPU_STOREOP_STORE;
    color_info.clear_color            = {0.02f, 0.02f, 0.03f, 1.0f};

    SDL_GPUDepthStencilTargetInfo depth_info = {};
    depth_info.texture                       = cubemap_depth.get();
    depth_info.load_op                       = SDL_GPU_LOADOP_CLEAR;
    depth_info.store_op                      = SDL_GPU_STOREOP_DONT_CARE;
    depth_info.stencil_load_op               = SDL_GPU_LOADOP_CLEAR;
    depth_info.stencil_store_op              = SDL_GPU_STOREOP_DONT_CARE;
    depth_info.clear_depth                   = 1.0f;
    depth_info.cycle                         = true;

    SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, &depth_info);

    SDL_GPUViewport vp = {0.0f, 0.0f, (float)CUBEMAP_SIZE, (float)CUBEMAP_SIZE, 0.0f, 1.0f};
    SDL_SetGPUViewport(pass, &vp);

    glm::vec3 const &cam_pos   = MIRROR_CENTERS[mirror_index];
    glm::mat4        face_view = glm::lookAt(glm::vec3(0.0f), FACE_TARGETS[face], FACE_UPS[face]);
    glm::mat4        proj      = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
    proj[0][0] *= -1.0f; // match the cubemap sampling convention (see inter_reflection)

    render_box(cmd, pass, cam_pos, face_view, proj);

    // Skip the other mirror on the first frame so the cubemaps are seeded with
    // the box + background before any mirror samples another mirror's cubemap.
    int const other = 1 - mirror_index;
    if (m_frame_count > 1)
        render_mirror(cmd, pass, cam_pos, face_view, proj, other, mirror_cubemaps[other].get());

    SDL_EndGPURenderPass(pass);
}

void scene_t::render_all_cubemap_faces(SDL_GPUCommandBuffer *cmd) {
    for (int i = 0; i < NUM_MIRRORS; ++i)
        for (int face = 0; face < 6; ++face)
            render_cubemap_face(cmd, i, face);
}

void scene_t::render_main(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 const view = camera.rotation_view();
    glm::mat4 const proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    render_box(cmd, pass, camera.position, view, proj);
    for (int i = 0; i < NUM_MIRRORS; ++i)
        render_mirror(cmd, pass, camera.position, view, proj, i, mirror_cubemaps[i].get());
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 27 - Infinity Mirror", WINDOW_WIDTH, WINDOW_HEIGHT, parse_engine_args(argc, argv)
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

    // The mirror cubemaps are rebuilt in prepare (outside the swapchain pass);
    // the main pass then samples them.
    std::array<pass_desc_t, 1> passes = {{
        {.depth_texture = &scene->main_depth.texture,
         .prepare =
             [&](SDL_GPUCommandBuffer *cmd) {
                 scene->render_all_cubemap_faces(cmd);
                 imgui_prepare(cmd);
             },
         .draw =
             [&](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
                 scene->render_main(cmd, pass);
                 imgui_render(cmd, pass);
             }},
    }};

    auto result = run_loop(
        *engine, [&]() { return SDL_FColor{0.02f, 0.02f, 0.03f, 1.0f}; }, scene->main_depth,
        scene->unused_color_target, [&](input_t const &in) { return scene->update(in); }, passes
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
