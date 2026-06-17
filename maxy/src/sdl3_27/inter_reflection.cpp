// inter_reflection.cpp — two textured cubes that reflect each other.
//
// The cubes are textured + lit (environment.frag) blended with a per-object
// reflection cubemap, so each cube is a visible object in the scene that ALSO
// mirrors its surroundings -- including the other cube.
//
// Each object owns ONE persistent cubemap. Every frame, in the prepare phase:
//   For each object i (6 render passes), render the scene from i's center into
//   i's cubemap. The OTHER objects are drawn sampling their own persistent
//   cubemap, which still holds their most recent render -- so object i reflects
//   the latest available image of object j without an infinite intra-frame
//   dependency. The main pass then samples the same persistent cubemaps.
//
// One persistent texture per object (rather than a swapped front/back pair) is
// deliberate: with double-buffering the mutual reflections settled into a
// two-frame limit cycle (a reflects b, b reflects a) that showed up as blinking.
//
// Seeding: the reflection component would sample an uninitialised cubemap on the
// very first frame, so inter-object reflections are skipped once at startup to
// populate every cubemap with the environment first. (The textured base colour
// keeps the cubes visible regardless, but this keeps the reflection clean.)

#include <array>
#include <format>
#include <print>
#include <span>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"
#include "lights.hpp"

constexpr int      WINDOW_WIDTH  = 1024;
constexpr int      WINDOW_HEIGHT = 768;
constexpr uint32_t CUBEMAP_SIZE  = 256;
constexpr int      NUM_OBJECTS   = 2;

// Cubemap face render orientations. Layer 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z.
static constexpr std::array<glm::vec3, 6> FACE_TARGETS = {{
    {1, 0, 0},
    {-1, 0, 0},
    {0, 1, 0},
    {0, -1, 0},
    {0, 0, 1},
    {0, 0, -1},
}};
// Up vectors that orient each face vertically for SDL3 GPU's Y-up clip space
// (the negation of the raw OpenGL convention, whose down-pointing up vectors
// would render the faces upside down here). Horizontal orientation is corrected
// separately by flipping the projection's X axis in render_cubemap_face.
static constexpr std::array<glm::vec3, 6> FACE_UPS = {{
    {0, 1, 0},
    {0, 1, 0}, // +X, -X
    {0, 0, -1},
    {0, 0, 1}, // +Y, -Y
    {0, 1, 0},
    {0, 1, 0}, // +Z, -Z
}};

struct scene_params_t {
    float     shininess;
    int       pos_count;
    int       spot_count;
    int       pad;
    glm::vec4 dir_direction;
    glm::vec4 dir_ambient;
    glm::vec4 dir_diffuse;
    glm::vec4 dir_specular;
};

// Cube positions and common size
static constexpr glm::vec3 OBJECT_POSITIONS[NUM_OBJECTS] = {
    {-1.5f, 0.5f, 0.0f},
    {1.5f, 0.5f, 0.0f},
};
static constexpr float OBJECT_SIZE = 1.0f;

static constexpr glm::vec3 POINT_LIGHT_POS = {0.0f, 2.5f, 0.0f};

// Skybox vertices: position IS the cubemap sample direction.
static constexpr auto SKYBOX_VERTICES = []() {
    std::array<vertex_t, 36> verts{};
    for (size_t i = 0; i < 36; ++i)
        verts[i] = unit_cube_with_normals[i].position;
    return verts;
}();

namespace {

// Loads 6 images as a static (read-only) cubemap texture.
// No vertical flip -- cubemap face orientation matches SDL3_image top-left origin.
std::expected<gpu_texture_t, std::string>
load_cubemap_texture(engine_t const &engine, std::span<std::string const, 6> face_paths) {
    using transfer_t = gpu_resource_t<SDL_GPUTransferBuffer, SDL_ReleaseGPUTransferBuffer>;

    std::array<SDL_Surface *, 6> surfaces{};
    int                          face_w = 0, face_h = 0;
    for (int i = 0; i < 6; ++i) {
        SDL_Surface *raw = IMG_Load(face_paths[i].c_str());
        if (!raw) {
            for (int j = 0; j < i; ++j)
                SDL_DestroySurface(surfaces[j]);
            return std::unexpected(
                std::format("IMG_Load failed for {}: {}", face_paths[i], SDL_GetError())
            );
        }
        surfaces[i] = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(raw);
        if (!surfaces[i]) {
            for (int j = 0; j < i; ++j)
                SDL_DestroySurface(surfaces[j]);
            return sdl_error("SDL_ConvertSurface failed");
        }
        if (i == 0) {
            face_w = surfaces[i]->w;
            face_h = surfaces[i]->h;
        }
    }

    Uint32 const face_size  = static_cast<Uint32>(face_w * face_h * 4);
    Uint32 const total_size = face_size * 6;

    SDL_GPUTextureCreateInfo tex_info = {};
    tex_info.type                     = SDL_GPU_TEXTURETYPE_CUBE;
    tex_info.format                   = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.usage                    = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tex_info.width                    = static_cast<Uint32>(face_w);
    tex_info.height                   = static_cast<Uint32>(face_h);
    tex_info.layer_count_or_depth     = 6;
    tex_info.num_levels               = 1;
    gpu_texture_t texture{engine.gpu_device, SDL_CreateGPUTexture(engine.gpu_device, &tex_info)};
    if (!texture) {
        for (auto *s : surfaces)
            SDL_DestroySurface(s);
        return sdl_error("SDL_CreateGPUTexture (static cubemap) failed");
    }

    SDL_GPUTransferBufferCreateInfo tbuf_info = {};
    tbuf_info.usage                           = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbuf_info.size                            = total_size;
    transfer_t transfer{
        engine.gpu_device, SDL_CreateGPUTransferBuffer(engine.gpu_device, &tbuf_info)
    };
    if (!transfer) {
        for (auto *s : surfaces)
            SDL_DestroySurface(s);
        return sdl_error("SDL_CreateGPUTransferBuffer failed");
    }

    auto *mapped =
        static_cast<Uint8 *>(SDL_MapGPUTransferBuffer(engine.gpu_device, transfer.get(), false));
    if (!mapped) {
        for (auto *s : surfaces)
            SDL_DestroySurface(s);
        return sdl_error("SDL_MapGPUTransferBuffer failed");
    }
    for (int i = 0; i < 6; ++i) {
        SDL_memcpy(mapped + i * face_size, surfaces[i]->pixels, face_size);
        SDL_DestroySurface(surfaces[i]);
    }
    SDL_UnmapGPUTransferBuffer(engine.gpu_device, transfer.get());

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!cmd) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUCopyPass *cp = SDL_BeginGPUCopyPass(cmd);
    for (Uint32 i = 0; i < 6; ++i) {
        SDL_GPUTextureTransferInfo src = {};
        src.transfer_buffer            = transfer.get();
        src.offset                     = i * face_size;
        src.pixels_per_row             = static_cast<Uint32>(face_w);
        src.rows_per_layer             = static_cast<Uint32>(face_h);
        SDL_GPUTextureRegion dst       = {};
        dst.texture                    = texture.get();
        dst.layer                      = i;
        dst.w                          = static_cast<Uint32>(face_w);
        dst.h                          = static_cast<Uint32>(face_h);
        dst.d                          = 1;
        SDL_UploadToGPUTexture(cp, &src, &dst, false);
    }
    SDL_EndGPUCopyPass(cp);
    if (!SDL_SubmitGPUCommandBuffer(cmd)) return sdl_error("SDL_SubmitGPUCommandBuffer failed");
    return texture;
}

// Creates a renderable+sampleable cube texture in the swapchain format so
// existing pipelines (created for the swapchain) can write into it.
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

} // namespace

// One reflective object: a single persistent cubemap that is re-rendered in
// place each frame and sampled both by the other objects and by the main view.
// Using one stable texture (rather than a swapped pair) keeps the mutual
// reflections from oscillating between two buffers, which looked like blinking.
struct reflective_t {
    glm::vec3     position;
    float         size;
    gpu_texture_t cubemap; // COLOR_TARGET | SAMPLER, persists between renders
    gpu_texture_t depth;   // reused for all 6 faces (rendered sequentially)
    gpu_sampler_t sampler;
};

struct scene_t {
    gpu_pipeline_t lit_pipeline;  // floor (Phong only)
    gpu_pipeline_t cube_pipeline; // textured-lit cubes blended with their reflection
    gpu_pipeline_t skybox_pipeline;

    gpu_geometry_t cube_geometry;
    gpu_geometry_t floor_geometry;
    gpu_geometry_t skybox_geometry;

    gpu_material_t cube_material;
    gpu_material_t floor_material;
    gpu_texture_t  static_cubemap;
    gpu_sampler_t  static_sampler;

    reflective_t objects[NUM_OBJECTS];

    // Dummy color target: required by the multi-pass run_loop API even when all
    // passes render to the swapchain. Never referenced by any pass_desc_t.
    tracked_color_target_t color_target;

    camera_t camera;
    float    m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    int      m_frame_count  = 0; // counts frames; used to seed the cubemaps

    bool update(input_t const &in);

    void push_lit_scene_uniforms(
        SDL_GPUCommandBuffer *cmd, glm::mat4 const &view, glm::mat4 const &proj,
        glm::vec3 const &cam_pos
    ) const;

    void render_floor(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
        glm::mat4 const &view, glm::mat4 const &proj
    ) const;

    void render_reflective_cube(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
        glm::mat4 const &view, glm::mat4 const &proj, int obj_idx, SDL_GPUTexture *cubemap_tex
    ) const;

    void render_skybox(
        SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::mat4 const &view_rotation,
        glm::mat4 const &proj
    ) const;

    void render_cubemap_face(SDL_GPUCommandBuffer *cmd, int obj_idx, int face);
    void render_all_cubemap_faces(SDL_GPUCommandBuffer *cmd);
    void render_scene(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window, {0.0f, 1.5f, 5.0f});

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGui::GetIO().IniFilename = nullptr;

    auto lit_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_24/lit.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 2,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!lit_pipe) return std::unexpected(lit_pipe.error());
    scene.lit_pipeline = std::move(*lit_pipe);

    // Cubes use environment.frag: Phong lighting on a textured material blended
    // with a cubemap reflection (diffuse/specular at samplers 0/1, the reflected
    // cubemap at sampler 2). Same shader as environment.cpp, but the cubemap is
    // each object's own dynamically-rendered one rather than the static skybox.
    auto cube_pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_24/lit.vert.spv",
                    .fragment_shader          = "shaders/sdl3_27/environment.frag.spv",
                    .vertex_uniform_buffers   = 4,
                    .fragment_uniform_buffers = 4,
                    .fragment_samplers        = 3,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                    // No culling: cubemap faces are rendered with a flipped-X
                    // projection (inverting winding), and a convex cube renders
                    // correctly without culling thanks to the depth test.
                    .cull_mode = SDL_GPU_CULLMODE_NONE,
                }
    );
    if (!cube_pipe) return std::unexpected(cube_pipe.error());
    scene.cube_pipeline = std::move(*cube_pipe);

    auto skybox_pipe = create_pipeline(
        engine, {
                    .vertex_shader          = "shaders/sdl3_27/skybox.vert.spv",
                    .fragment_shader        = "shaders/sdl3_27/skybox.frag.spv",
                    .vertex_uniform_buffers = 2,
                    .fragment_samplers      = 1,
                    .enable_depth_test      = true,
                    .depth_compare_op       = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                    .enable_depth_write     = false,
                }
    );
    if (!skybox_pipe) return std::unexpected(skybox_pipe.error());
    scene.skybox_pipeline = std::move(*skybox_pipe);

    auto cube_geom = create_vertex_geometry(
        engine, unit_cube_with_normals.data(),
        static_cast<Uint32>(unit_cube_with_normals.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(unit_cube_with_normals.size())
    );
    if (!cube_geom) return std::unexpected(cube_geom.error());
    scene.cube_geometry = std::move(*cube_geom);

    static constexpr std::array<pos_normal_uv_vertex_t, 6> floor_verts = {{
        {{500, 0, 500}, {0, 1, 0}, {200, 0}},
        {{-500, 0, -500}, {0, 1, 0}, {0, 200}},
        {{-500, 0, 500}, {0, 1, 0}, {0, 0}},
        {{500, 0, 500}, {0, 1, 0}, {200, 0}},
        {{500, 0, -500}, {0, 1, 0}, {200, 200}},
        {{-500, 0, -500}, {0, 1, 0}, {0, 200}},
    }};
    auto                                                   floor_geom  = create_vertex_geometry(
        engine, floor_verts.data(),
        static_cast<Uint32>(floor_verts.size() * sizeof(pos_normal_uv_vertex_t)),
        static_cast<Uint32>(floor_verts.size())
    );
    if (!floor_geom) return std::unexpected(floor_geom.error());
    scene.floor_geometry = std::move(*floor_geom);

    auto skybox_geom = create_vertex_geometry(
        engine, SKYBOX_VERTICES.data(),
        static_cast<Uint32>(SKYBOX_VERTICES.size() * sizeof(vertex_t)),
        static_cast<Uint32>(SKYBOX_VERTICES.size())
    );
    if (!skybox_geom) return std::unexpected(skybox_geom.error());
    scene.skybox_geometry = std::move(*skybox_geom);

    auto cube_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/container2.png",
                     std::string(ASSETS_PATH) + "textures/container2_specular.png",
                 }}
    );
    if (!cube_mat) return std::unexpected(cube_mat.error());
    scene.cube_material = std::move(*cube_mat);

    auto floor_mat = create_material(
        engine, {.texture_paths = {
                     std::string(ASSETS_PATH) + "textures/metal.png",
                     std::string(ASSETS_PATH) + "textures/metal.png",
                 }}
    );
    if (!floor_mat) return std::unexpected(floor_mat.error());
    scene.floor_material = std::move(*floor_mat);

    std::array<std::string, 6> face_paths = {{
        std::string(ASSETS_PATH) + "textures/skybox/right.jpg",
        std::string(ASSETS_PATH) + "textures/skybox/left.jpg",
        std::string(ASSETS_PATH) + "textures/skybox/top.jpg",
        std::string(ASSETS_PATH) + "textures/skybox/bottom.jpg",
        std::string(ASSETS_PATH) + "textures/skybox/front.jpg",
        std::string(ASSETS_PATH) + "textures/skybox/back.jpg",
    }};
    auto static_cm = load_cubemap_texture(engine, std::span<std::string const, 6>(face_paths));
    if (!static_cm) return std::unexpected(static_cm.error());
    scene.static_cubemap = std::move(*static_cm);

    auto static_samp = create_sampler(engine, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
    if (!static_samp) return std::unexpected(static_samp.error());
    scene.static_sampler = std::move(*static_samp);

    // Build reflective objects with one persistent dynamic cubemap each.
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        scene.objects[i].position = OBJECT_POSITIONS[i];
        scene.objects[i].size     = OBJECT_SIZE;

        auto cm = create_dynamic_cubemap(engine);
        if (!cm) return std::unexpected(cm.error());
        scene.objects[i].cubemap = std::move(*cm);

        auto dep = create_depth_texture(engine, CUBEMAP_SIZE, CUBEMAP_SIZE);
        if (!dep) return std::unexpected(dep.error());
        scene.objects[i].depth = std::move(*dep);

        auto samp = create_sampler(engine, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
        if (!samp) return std::unexpected(samp.error());
        scene.objects[i].sampler = std::move(*samp);
    }

    auto dummy_ct = create_tracked_color_target(engine);
    if (!dummy_ct) return std::unexpected(dummy_ct.error());
    scene.color_target = std::move(*dummy_ct);

    return scene;
}

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.update(in);

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::End();

    ++m_frame_count;
    return true;
}

// Pushes scene-wide lit shader uniforms (view, proj, pos light, flashlight).
// The caller still needs to push the model and normal_flip vertex uniforms
// (slots 0 and 3) and call draw() to bind material textures.
void scene_t::push_lit_scene_uniforms(
    SDL_GPUCommandBuffer *cmd, glm::mat4 const &view, glm::mat4 const &proj,
    glm::vec3 const &cam_pos
) const {
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, proj);

    scene_params_t sp = {
        .shininess     = 64.0f,
        .pos_count     = 1,
        .spot_count    = 0,
        .dir_direction = glm::vec4(glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)), 0.0f),
        .dir_ambient   = glm::vec4(0.15f, 0.15f, 0.15f, 0.0f),
        .dir_diffuse   = glm::vec4(0.6f, 0.6f, 0.6f, 0.0f),
        .dir_specular  = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f),
    };
    push_fragment_uniform(cmd, 0, sp);

    pos_lights_block_t<MAX_POS_LIGHTS> pos_block{};
    pos_block.lights[0] = {
        .position  = glm::vec4(POINT_LIGHT_POS - cam_pos, 0.0f),
        .ambient   = glm::vec4(0.05f, 0.05f, 0.05f, 0.0f),
        .diffuse   = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f),
        .specular  = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f),
        .constant  = 1.0f,
        .linear    = 0.09f,
        .quadratic = 0.032f,
    };
    push_fragment_uniform(cmd, 1, pos_block);
    push_fragment_uniform(cmd, 2, spot_lights_block_t<MAX_SPOT_LIGHTS>{});
    // constant=0 → attenuation = 1/0 = INF; INF * 0 (black colours) = NaN.
    flashlight_uniforms_t empty_flashlight{};
    empty_flashlight.constant = 1.0f;
    push_fragment_uniform(cmd, 3, empty_flashlight);
}

void scene_t::render_floor(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
    glm::mat4 const &view, glm::mat4 const &proj
) const {
    SDL_BindGPUGraphicsPipeline(pass, lit_pipeline.get());
    push_lit_scene_uniforms(cmd, view, proj, cam_pos);
    auto model = glm::translate(glm::mat4{1.0f}, -cam_pos);
    push_vertex_uniform(cmd, 0, model);
    push_vertex_uniform(cmd, 3, 1.0f);
    draw(floor_geometry, floor_material, pass);
}

void scene_t::render_reflective_cube(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::vec3 const &cam_pos,
    glm::mat4 const &view, glm::mat4 const &proj, int obj_idx, SDL_GPUTexture *cubemap_tex
) const {
    auto const &obj = objects[obj_idx];
    auto        model =
        glm::scale(glm::translate(glm::mat4{1.0f}, obj.position - cam_pos), glm::vec3{obj.size});

    push_lit_scene_uniforms(cmd, view, proj, cam_pos);
    push_vertex_uniform(cmd, 0, model);
    push_vertex_uniform(cmd, 3, 1.0f);

    // The reflected cubemap goes at fragment sampler slot 2; draw() fills slots
    // 0 and 1 from the cube material (diffuse + specular).
    SDL_GPUTextureSamplerBinding env_binding = {cubemap_tex, obj.sampler.get()};
    SDL_BindGPUFragmentSamplers(pass, 2, &env_binding, 1);
    draw(cube_geometry, cube_material, pass);
}

void scene_t::render_skybox(
    SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass, glm::mat4 const &view_rotation,
    glm::mat4 const &proj
) const {
    SDL_BindGPUGraphicsPipeline(pass, skybox_pipeline.get());
    push_vertex_uniform(cmd, 0, view_rotation);
    push_vertex_uniform(cmd, 1, proj);

    SDL_GPUTextureSamplerBinding binding = {static_cubemap.get(), static_sampler.get()};
    SDL_BindGPUFragmentSamplers(pass, 0, &binding, 1);

    SDL_GPUBufferBinding vb = {skybox_geometry.vertex_buffer.get(), 0};
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);
    SDL_DrawGPUPrimitives(pass, skybox_geometry.vertex_count, 1, 0, 0);
}

// Renders the scene as seen from object obj_idx looking toward face direction.
// Other reflective objects are sampled from their persistent cubemaps.
void scene_t::render_cubemap_face(SDL_GPUCommandBuffer *cmd, int obj_idx, int face) {
    auto const &obj = objects[obj_idx];

    SDL_GPUColorTargetInfo color_info = {};
    color_info.texture                = obj.cubemap.get();
    color_info.layer_or_depth_plane   = static_cast<Uint32>(face);
    color_info.load_op                = SDL_GPU_LOADOP_CLEAR;
    color_info.store_op               = SDL_GPU_STOREOP_STORE;
    color_info.clear_color            = {0.0f, 0.0f, 0.0f, 1.0f};

    // One depth texture is reused for all 6 face passes. Cycling gives each pass
    // a fresh internal depth buffer, avoiding the write-after-write hazard of
    // overwriting depth data referenced by the previous face's draws. Cycling a
    // depth/stencil target requires its stencil load op to be CLEAR (not LOAD).
    SDL_GPUDepthStencilTargetInfo depth_info = {};
    depth_info.texture                       = obj.depth.get();
    depth_info.load_op                       = SDL_GPU_LOADOP_CLEAR;
    depth_info.store_op                      = SDL_GPU_STOREOP_DONT_CARE;
    depth_info.stencil_load_op               = SDL_GPU_LOADOP_CLEAR;
    depth_info.stencil_store_op              = SDL_GPU_STOREOP_DONT_CARE;
    depth_info.clear_depth                   = 1.0f;
    depth_info.cycle                         = true;

    SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, &depth_info);

    // The cubemap face is CUBEMAP_SIZE x CUBEMAP_SIZE; set the viewport explicitly
    // so it doesn't inherit the window's dimensions.
    SDL_GPUViewport vp = {0.0f, 0.0f, (float)CUBEMAP_SIZE, (float)CUBEMAP_SIZE, 0.0f, 1.0f};
    SDL_SetGPUViewport(pass, &vp);

    glm::vec3 const &cam_pos = obj.position;
    // View looks along face direction from object center (no translation: the
    // model matrices bake in -cam_pos, matching the convention used elsewhere).
    glm::mat4 face_view = glm::lookAt(glm::vec3(0.0f), FACE_TARGETS[face], FACE_UPS[face]);
    glm::mat4 proj      = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
    // SDL3 GPU's clip space is right-handed (Y up), but cubemap faces are
    // sampled in a left-handed convention. Flipping the projection's X axis
    // makes the rendered faces match what the cubemap sampler expects, so
    // reflections are not horizontally mirrored. This inverts triangle winding,
    // which is why the cube pipeline disables back-face culling.
    proj[0][0] *= -1.0f;

    render_floor(cmd, pass, cam_pos, face_view, proj);

    // Skip inter-object reflections on the first frame so every cubemap is
    // seeded with the environment before any object samples another's cubemap
    // (which would otherwise be uninitialised). From then on, each object draws
    // the OTHER objects sampling their persistent cubemap, which holds that
    // object's most recent render.
    bool const cubemaps_are_seeded = m_frame_count > 1;
    if (cubemaps_are_seeded) {
        SDL_BindGPUGraphicsPipeline(pass, cube_pipeline.get());
        for (int j = 0; j < NUM_OBJECTS; ++j) {
            if (j == obj_idx) continue;
            render_reflective_cube(
                cmd, pass, cam_pos, face_view, proj, j, objects[j].cubemap.get()
            );
        }
    }

    glm::mat4 skybox_view = glm::mat4(glm::mat3(face_view));
    render_skybox(cmd, pass, skybox_view, proj);

    SDL_EndGPURenderPass(pass);
}

void scene_t::render_all_cubemap_faces(SDL_GPUCommandBuffer *cmd) {
    for (int i = 0; i < NUM_OBJECTS; ++i)
        for (int face = 0; face < 6; ++face)
            render_cubemap_face(cmd, i, face);
}

void scene_t::render_scene(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 view = camera.rotation_view();
    glm::mat4 proj = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    render_floor(cmd, pass, camera.position, view, proj);

    // Render reflective cubes using their freshly-built cubemaps.
    SDL_BindGPUGraphicsPipeline(pass, cube_pipeline.get());
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        render_reflective_cube(cmd, pass, camera.position, view, proj, i, objects[i].cubemap.get());
    }

    render_skybox(cmd, pass, glm::mat4(glm::mat3(view)), proj);
}

int main(int argc, char *argv[]) {
    auto engine = create_engine(
        "SDL3 27 - Inter-Object Reflections", WINDOW_WIDTH, WINDOW_HEIGHT,
        parse_engine_args(argc, argv)
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

    auto depth = create_tracked_depth(*engine);
    if (!depth) {
        std::println(stderr, "{}", depth.error());
        return 1;
    }

    // Cubemap rendering happens in prepare (outside the swapchain render pass),
    // so that render_scene can immediately sample the freshly-built cubemaps.
    std::array<pass_desc_t, 1> passes = {{
        {.depth_texture = &depth->texture,
         .prepare =
             [&](SDL_GPUCommandBuffer *cmd) {
                 scene->render_all_cubemap_faces(cmd);
                 imgui_prepare(cmd);
             },
         .draw =
             [&](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
                 scene->render_scene(cmd, pass);
                 imgui_render(cmd, pass);
             }},
    }};

    auto result = run_loop(
        *engine, [&]() { return SDL_FColor{0.1f, 0.1f, 0.1f, 1.0f}; }, *depth, scene->color_target,
        [&](input_t const &in) { return scene->update(in); }, passes
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
