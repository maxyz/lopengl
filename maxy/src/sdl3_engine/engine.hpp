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

// Returns false when the application should quit (window closed).
// Games handle their own exit conditions (escape, menus, etc.) separately.
bool poll_events();

// Returns seconds elapsed since the last call. Call once per frame.
// Returns 0 on the first call.
float tick(engine_t &engine);

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
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

// Aggregates a pipeline, vertex/index buffers, textures, and a sampler for
// indexed textured drawing. All resources are owned via RAII wrappers.
struct textured_mesh_t {
    gpu_pipeline_t             pipeline;
    gpu_buffer_t               vertex_buffer;
    gpu_buffer_t               index_buffer;
    std::vector<gpu_texture_t> textures;
    std::vector<gpu_sampler_t> samplers; // one per texture
    Uint32                     index_count;
};

struct textured_mesh_desc_t {
    std::string_view                                vertex_shader;
    std::string_view                                fragment_shader;
    Uint32                                          vertex_uniform_buffers   = 0;
    Uint32                                          fragment_uniform_buffers = 0;
    std::span<SDL_GPUVertexBufferDescription const> vertex_buffer_descs      = {};
    std::span<SDL_GPUVertexAttribute const>         vertex_attributes        = {};
    void const                                     *vertices;
    Uint32                                          vertex_data_size;
    std::span<uint16_t const>                       indices;
    std::vector<std::string>                        texture_paths;
    // One address mode per texture. If shorter than texture_paths, remaining
    // textures use SDL_GPU_SAMPLERADDRESSMODE_REPEAT.
    std::vector<SDL_GPUSamplerAddressMode> address_modes;
    // One filter mode per texture. If shorter, remaining textures use LINEAR.
    std::vector<SDL_GPUFilter> filter_modes;
};

std::expected<textured_mesh_t, std::string>
create_textured_mesh(engine_t const &engine, textured_mesh_desc_t desc);

// Binds the pipeline, buffers, textures, and sampler, then draws.
void draw(textured_mesh_t const &mesh, SDL_GPURenderPass *pass);
