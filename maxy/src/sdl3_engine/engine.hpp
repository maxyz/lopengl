#pragma once
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include <SDL3/SDL.h>

// Generic RAII owner for any SDL GPU object released via Release(device,
// handle). Move-only; destructor fires the release call.
template <typename T, void (*Release)(SDL_GPUDevice *, T *)> struct gpu_resource_t {
    SDL_GPUDevice *device = nullptr;
    T *handle = nullptr;

    gpu_resource_t() = default;
    gpu_resource_t(SDL_GPUDevice *dev, T *h) : device(dev), handle(h) {}

    gpu_resource_t(gpu_resource_t const &) = delete;
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

    T *get() const { return handle; }
    explicit operator bool() const { return handle != nullptr; }
};

using gpu_pipeline_t = gpu_resource_t<SDL_GPUGraphicsPipeline, SDL_ReleaseGPUGraphicsPipeline>;
using gpu_buffer_t = gpu_resource_t<SDL_GPUBuffer, SDL_ReleaseGPUBuffer>;
using gpu_shader_t = gpu_resource_t<SDL_GPUShader, SDL_ReleaseGPUShader>;

struct engine_config_t {
    bool verbose = false;
};

engine_config_t parse_engine_args(int argc, char *argv[]);

std::unexpected<std::string> sdl_error(std::string prefix);

struct engine_t {
    SDL_Window *window = nullptr;
    SDL_GPUDevice *gpu_device = nullptr;
    bool sdl_initialized = false;
    bool verbose = false;
    Uint64 last_tick = 0;

    engine_t() = default;
    engine_t(engine_t const &) = delete;
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

// Describes the shaders and resource bindings for a graphics pipeline.
// Vertex layout is fixed: one vertex_t (float3 position) at location 0.
struct pipeline_desc_t {
    std::string_view vertex_shader;
    std::string_view fragment_shader;
    Uint32 num_uniform_buffers = 0; // vertex stage
    Uint32 num_samplers        = 0; // fragment stage
};

std::expected<gpu_pipeline_t, std::string>
create_pipeline(engine_t const &engine, pipeline_desc_t const &desc);
