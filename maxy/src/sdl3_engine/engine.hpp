#pragma once
#include <expected>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include "geometry.hpp"

// Generic RAII owner for any SDL GPU object released via Release(device,
// handle). Move-only; destructor fires the release call.
template <typename T, void (*Release)(SDL_GPUDevice *, T *)> struct gpu_resource_t {
    SDL_GPUDevice *device = nullptr;
    T             *handle = nullptr;

    gpu_resource_t() = default;
    gpu_resource_t(SDL_GPUDevice *dev, T *h) : device(dev), handle(h) {}

    gpu_resource_t(gpu_resource_t const &)            = delete;
    gpu_resource_t &operator=(gpu_resource_t const &) = delete;

    gpu_resource_t(gpu_resource_t &&other) noexcept
        : device(std::exchange(other.device, nullptr)),
          handle(std::exchange(other.handle, nullptr)) {}

    gpu_resource_t &operator=(gpu_resource_t &&other) noexcept {
        if (this != &other) {
            if (device && handle) Release(device, handle);
            device = std::exchange(other.device, nullptr);
            handle = std::exchange(other.handle, nullptr);
        }
        return *this;
    }

    ~gpu_resource_t() {
        if (device && handle) Release(device, handle);
    }

    T       *get() const { return handle; }
    explicit operator bool() const { return handle != nullptr; }
};

using gpu_pipeline_t = gpu_resource_t<SDL_GPUGraphicsPipeline, SDL_ReleaseGPUGraphicsPipeline>;
using gpu_buffer_t   = gpu_resource_t<SDL_GPUBuffer, SDL_ReleaseGPUBuffer>;
using gpu_shader_t   = gpu_resource_t<SDL_GPUShader, SDL_ReleaseGPUShader>;
using gpu_texture_t  = gpu_resource_t<SDL_GPUTexture, SDL_ReleaseGPUTexture>;
using gpu_sampler_t  = gpu_resource_t<SDL_GPUSampler, SDL_ReleaseGPUSampler>;

struct engine_config_t {
    bool verbose = false;
};

engine_config_t parse_engine_args(int argc, char *argv[]);

std::unexpected<std::string> sdl_error(std::string prefix);

struct engine_t {
    SDL_Window    *window          = nullptr;
    SDL_GPUDevice *gpu_device      = nullptr;
    bool           sdl_initialized = false;
    bool           verbose         = false;
    Uint64         last_tick       = 0;

    engine_t()                            = default;
    engine_t(engine_t const &)            = delete;
    engine_t &operator=(engine_t const &) = delete;
    engine_t(engine_t &&) noexcept;
    engine_t &operator=(engine_t &&) noexcept;
    ~engine_t();
};

std::expected<engine_t, std::string>
create_engine(std::string_view title, int width, int height, engine_config_t const &config = {});

// Initialise Dear ImGui with the SDL3 platform and SDL_GPU renderer backends.
// Call from create_scene before configuring fonts; the engine destructor runs shutdown.
std::expected<void, std::string> init_imgui(engine_t const &engine);

// Returns false when the application should quit (window closed).
// Games handle their own exit conditions (escape, menus, etc.) separately.
bool poll_events();

// Returns seconds elapsed since the last call. Call once per frame.
// Returns 0 on the first call.
float tick(engine_t &engine);

// Returns the window size in pixels. Use this to detect resizes and recompute
// aspect ratios or recreate size-dependent resources (e.g. depth textures).
glm::ivec2 window_pixel_size(engine_t const &engine);

// Returns width / height. Divide clip-space x by this to preserve proportions.
// Superseded by a proper projection matrix from chapter 8 onward.
float aspect_ratio(engine_t const &engine);

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
);

// render_frame with depth -- depth_texture must be created with create_depth_texture.
// Calls imgui_prepare_draw_data before the render pass when an ImGui context exists.
std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color, gpu_texture_t const &depth_texture,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
);

// Reads a SPIR-V file and creates a GPU shader stage.
// num_uniform_buffers and num_samplers must match the shader's declared bindings.
std::expected<gpu_shader_t, std::string> load_shader(
    engine_t const &engine, std::string_view spv_path, SDL_GPUShaderStage stage,
    Uint32 num_uniform_buffers = 0, Uint32 num_samplers = 0
);

// Allocates a GPU vertex buffer and uploads data via a one-shot copy pass.
std::expected<gpu_buffer_t, std::string>
create_vertex_buffer(engine_t const &engine, void const *data, Uint32 size);

// Allocates a GPU index buffer and uploads data via a one-shot copy pass.
std::expected<gpu_buffer_t, std::string>
create_index_buffer(engine_t const &engine, void const *data, Uint32 size);

// Creates a depth texture for 3D rendering. Recreate on window resize.
std::expected<gpu_texture_t, std::string>
create_depth_texture(engine_t const &engine, int width, int height);

// Depth texture that automatically recreates itself when the window is resized.
// Call update() once per frame before rendering; pass texture to render_frame.
struct tracked_depth_t {
    gpu_texture_t texture;
    glm::ivec2    size;

    // Recreates the texture if the window pixel size has changed.
    // Returns false if recreation failed (old texture remains valid).
    bool update(engine_t const &engine);
};

std::expected<tracked_depth_t, std::string> create_tracked_depth(engine_t const &engine);

// Loads an image file via SDL3_image and uploads it to a GPU texture.
std::expected<gpu_texture_t, std::string>
load_texture(engine_t const &engine, std::string_view path);

// Creates a sampler with linear filtering and the given address mode (default: repeat).
std::expected<gpu_sampler_t, std::string> create_sampler(
    engine_t const           &engine,
    SDL_GPUSamplerAddressMode address_mode = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    SDL_GPUFilter             filter       = SDL_GPU_FILTER_LINEAR
);

// Describes the shaders and resource bindings for a graphics pipeline.
// Vertex layout is fixed: one vertex_t (float3 position) at location 0.
struct pipeline_desc_t {
    std::string_view vertex_shader;
    std::string_view fragment_shader;
    Uint32           vertex_uniform_buffers   = 0;
    Uint32           fragment_uniform_buffers = 0;
    Uint32           fragment_samplers        = 0;
    // When empty, defaults to one vertex_t (float3) at location 0.
    std::span<SDL_GPUVertexBufferDescription const> vertex_buffer_descs = {};
    std::span<SDL_GPUVertexAttribute const>         vertex_attributes   = {};
    bool enable_depth_test = false; // enables depth test + write with LESS compare op
};

std::expected<gpu_pipeline_t, std::string>
create_pipeline(engine_t const &engine, pipeline_desc_t const &desc);

// Push a uniform value into the command buffer for the next draw call.
// Works for float, glm::vec4, glm::mat4, SDL_FColor, and any other type
// whose sizeof() matches its std140 size.
template <typename T> void push_vertex_uniform(SDL_GPUCommandBuffer *cmd, Uint32 slot, T const &v) {
    SDL_PushGPUVertexUniformData(cmd, slot, &v, sizeof(T));
}

template <typename T>
void push_fragment_uniform(SDL_GPUCommandBuffer *cmd, Uint32 slot, T const &v) {
    SDL_PushGPUFragmentUniformData(cmd, slot, &v, sizeof(T));
}

// glm::vec3 overloads: std140 requires 16-byte alignment for vec3, but
// sizeof(glm::vec3) is only 12. These overloads pad to vec4 transparently.
inline void push_vertex_uniform(SDL_GPUCommandBuffer *cmd, Uint32 slot, glm::vec3 const &v) {
    glm::vec4 padded{v, 0.0f};
    SDL_PushGPUVertexUniformData(cmd, slot, &padded, sizeof(glm::vec4));
}

inline void push_fragment_uniform(SDL_GPUCommandBuffer *cmd, Uint32 slot, glm::vec3 const &v) {
    glm::vec4 padded{v, 0.0f};
    SDL_PushGPUFragmentUniformData(cmd, slot, &padded, sizeof(glm::vec4));
}

// Vertex and index buffers for a single drawable piece of geometry.
struct gpu_geometry_t {
    gpu_buffer_t            vertex_buffer;
    gpu_buffer_t            index_buffer;           // empty when drawing without indices
    Uint32                  index_count        = 0; // > 0: SDL_DrawGPUIndexedPrimitives
    Uint32                  vertex_count       = 0; // > 0: SDL_DrawGPUPrimitives (non-indexed)
    SDL_GPUIndexElementSize index_element_size = SDL_GPU_INDEXELEMENTSIZE_16BIT;
};

// Uploads vertices and 16-bit indices to the GPU in one shot.
std::expected<gpu_geometry_t, std::string> create_geometry(
    engine_t const &engine, void const *vertices, Uint32 vertex_size,
    std::span<uint16_t const> indices
);

// Uploads vertices and 32-bit indices to the GPU in one shot.
// Use when index values exceed 65535 (e.g. large loaded models).
std::expected<gpu_geometry_t, std::string> create_geometry(
    engine_t const &engine, void const *vertices, Uint32 vertex_size,
    std::span<uint32_t const> indices
);

// Uploads vertices only (no index buffer) for glDrawArrays-style drawing.
std::expected<gpu_geometry_t, std::string> create_vertex_geometry(
    engine_t const &engine, void const *vertices, Uint32 vertex_size, Uint32 vertex_count
);

// Textures and their paired samplers for a draw call.
struct gpu_material_t {
    std::vector<gpu_texture_t> textures;
    std::vector<gpu_sampler_t> samplers; // one per texture
};

struct material_desc_t {
    std::vector<std::string> texture_paths;
    // One per texture; if shorter, remaining textures use REPEAT / LINEAR.
    std::vector<SDL_GPUSamplerAddressMode> address_modes;
    std::vector<SDL_GPUFilter>             filter_modes;
};

std::expected<gpu_material_t, std::string>
create_material(engine_t const &engine, material_desc_t desc);

// A pipeline, geometry, and material assembled into one drawable unit.
struct textured_mesh_t {
    gpu_pipeline_t pipeline;
    gpu_geometry_t geometry;
    gpu_material_t material;
};

// Bind all parts and issue the indexed draw call.
void draw(
    gpu_pipeline_t const &pipeline, gpu_geometry_t const &geometry, gpu_material_t const &material,
    SDL_GPURenderPass *pass
);

// Convenience overload for a pre-assembled mesh.
inline void draw(textured_mesh_t const &mesh, SDL_GPURenderPass *pass) {
    draw(mesh.pipeline, mesh.geometry, mesh.material, pass);
}

// Per-frame input snapshot delivered by run_loop to the update callback.
// dy is already negated for screen-Y-down convention.
struct input_t {
    bool const *keys;
    float       dx;
    float       dy;
    float       scroll;
    float       dt;
    float       aspect_ratio;
};

// Self-contained event loop: handles SDL events, focus tracking, relative mouse
// mode, depth texture management, tick, and ImGui integration when a context exists.
std::expected<void, std::string> run_loop(
    engine_t &engine, SDL_FColor clear_color, std::function<bool(input_t const &)> update,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
);

// Variant that queries the clear colour each frame — use when the background changes at runtime.
std::expected<void, std::string> run_loop(
    engine_t &engine, std::function<SDL_FColor()> get_clear_color,
    std::function<bool(input_t const &)>                             update,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
);

// FPS camera with Euler angles. Derives front/right/up axes on every update.
// process_mouse expects dy already negated for screen-Y-down convention.
struct camera_t {
    glm::vec3 position    = {0.0f, 0.0f, 3.0f};
    float     yaw         = -90.0f; // -90 so initial front points along -Z
    float     pitch       = 0.0f;
    float     fov         = 45.0f;
    float     speed       = 2.5f;
    float     sensitivity = 0.1f;
    bool      fps_mode    = false; // constrain W/S/A/D to XZ plane; disables R/F

    camera_t();
    camera_t(glm::vec3 position, float yaw = -90.0f, float pitch = 0.0f);
    // Construct with a window so update() can manage relative mouse mode automatically.
    camera_t(
        SDL_Window *window, glm::vec3 position = {0.0f, 0.0f, 3.0f}, float yaw = -90.0f,
        float pitch = 0.0f
    );

    glm::vec3 const &front() const { return m_front; }
    glm::vec3 const &right() const { return m_right; }
    glm::vec3 const &up() const { return m_up; }
    bool             ui_mode() const { return m_ui_mode; }

    glm::mat4 view_matrix() const;
    // Rotation-only view matrix for camera-relative world space rendering.
    // Use with model matrices built from (world_pos - camera.position) so
    // the view transform contributes only orientation, not translation.
    glm::mat4 rotation_view() const;

    // Handles the grave-key UI/fly toggle, relative mouse mode, and per-frame
    // camera input in a single call. Replaces the 12-line boilerplate that was
    // copy-pasted into every scene update().
    void update(input_t const &in);

    // Apply mouse delta (pixels). Pass dy already negated for screen-Y-down.
    void process_mouse(float dx, float dy);

    // Adjust FOV by scroll wheel delta; clamped to [1, 90].
    void process_scroll(float dy);

    // WASD forward/back/strafe + R/F up/down from SDL keyboard state.
    void process_keys(bool const *keys, float dt);

private:
    glm::vec3   m_front      = {0.0f, 0.0f, -1.0f};
    glm::vec3   m_right      = {1.0f, 0.0f, 0.0f};
    glm::vec3   m_up         = {0.0f, 1.0f, 0.0f};
    glm::vec3   m_world_up   = {0.0f, 1.0f, 0.0f};
    SDL_Window *m_window     = nullptr;
    bool        m_ui_mode    = false;
    bool        m_prev_grave = false;

    void update_vectors();
};

// Vertex layout for pos_uv_vertex_t: one buffer slot, float3 position at location 0,
// float2 uv at location 1. Pass to pipeline_desc_t::vertex_buffer_descs / vertex_attributes.
inline constexpr SDL_GPUVertexBufferDescription pos_uv_buffer_descs[] = {{
    .slot       = 0,
    .pitch      = sizeof(pos_uv_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
}};

inline constexpr SDL_GPUVertexAttribute pos_uv_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_uv_vertex_t, position))},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
     .offset      = static_cast<Uint32>(offsetof(pos_uv_vertex_t, uv))},
};

// Vertex layout for pos_normal_uv_vertex_t: float3 position (loc 0), float3 normal (loc 1),
// float2 UV (loc 2). Pitch = 32 bytes.
inline constexpr SDL_GPUVertexBufferDescription pos_normal_uv_buffer_descs[] = {{
    .slot       = 0,
    .pitch      = sizeof(pos_normal_uv_vertex_t),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
}};

inline constexpr SDL_GPUVertexAttribute pos_normal_uv_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, position))},
    {.location    = 1,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, normal))},
    {.location    = 2,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, uv))},
};

// Absorbs engine creation, scene creation, and run_loop into one call.
// CreateSceneFn takes engine_t& and returns std::expected<SceneT, std::string>.
// SceneT must have update(input_t const&) -> bool and render(cmd, pass).
template <typename CreateSceneFn>
std::expected<void, std::string> run_app(
    int argc, char *argv[], std::string_view title, int width, int height, SDL_FColor clear_color,
    CreateSceneFn &&create_scene_fn
) {
    auto engine = create_engine(title, width, height, parse_engine_args(argc, argv));
    if (!engine) return std::unexpected(engine.error());
    auto scene = create_scene_fn(*engine);
    if (!scene) return std::unexpected(scene.error());
    return run_loop(
        *engine, clear_color, [&](input_t const &in) { return scene->update(in); },
        [&](SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) { scene->render(cmd, pass); }
    );
}
