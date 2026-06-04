#include "engine.hpp"
#include "geometry.hpp"

#include <format>
#include <fstream>
#include <utility>
#include <vector>

std::unexpected<std::string> sdl_error(std::string prefix) {
    return std::unexpected(std::format("{}: {}", prefix, SDL_GetError()));
}

namespace {

std::expected<gpu_buffer_t, std::string> create_buffer(
    engine_t const &engine, SDL_GPUBufferUsageFlags usage, void const *data, Uint32 size
) {
    using transfer_t = gpu_resource_t<SDL_GPUTransferBuffer, SDL_ReleaseGPUTransferBuffer>;

    SDL_GPUBufferCreateInfo buffer_info = {};
    buffer_info.usage = usage;
    buffer_info.size = size;
    gpu_buffer_t buffer{engine.gpu_device, SDL_CreateGPUBuffer(engine.gpu_device, &buffer_info)};
    if (!buffer) return sdl_error("SDL_CreateGPUBuffer failed");

    SDL_GPUTransferBufferCreateInfo transfer_info = {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = size;
    transfer_t transfer{
        engine.gpu_device, SDL_CreateGPUTransferBuffer(engine.gpu_device, &transfer_info)
    };
    if (!transfer) return sdl_error("SDL_CreateGPUTransferBuffer failed");

    void *mapped = SDL_MapGPUTransferBuffer(engine.gpu_device, transfer.get(), false);
    if (!mapped) return sdl_error("SDL_MapGPUTransferBuffer failed");
    SDL_memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(engine.gpu_device, transfer.get());

    SDL_GPUCommandBuffer *cmd_buf = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!cmd_buf) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTransferBufferLocation source = {};
    source.transfer_buffer = transfer.get();

    SDL_GPUBufferRegion destination = {};
    destination.buffer = buffer.get();
    destination.size = size;

    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);
    SDL_EndGPUCopyPass(copy_pass);

    if (!SDL_SubmitGPUCommandBuffer(cmd_buf)) return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return buffer;
}

} // namespace

engine_t::engine_t(engine_t &&other) noexcept
    : window(std::exchange(other.window, nullptr)),
      gpu_device(std::exchange(other.gpu_device, nullptr)),
      sdl_initialized(std::exchange(other.sdl_initialized, false)), verbose(other.verbose),
      last_tick(other.last_tick) {}

engine_t &engine_t::operator=(engine_t &&other) noexcept {
    if (this != &other) {
        this->~engine_t();
        window = std::exchange(other.window, nullptr);
        gpu_device = std::exchange(other.gpu_device, nullptr);
        sdl_initialized = std::exchange(other.sdl_initialized, false);
        verbose = other.verbose;
        last_tick = other.last_tick;
    }
    return *this;
}

engine_t::~engine_t() {
    if (gpu_device && window) SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    if (gpu_device) SDL_DestroyGPUDevice(gpu_device);
    if (window) SDL_DestroyWindow(window);
    if (sdl_initialized) SDL_Quit();
}

engine_config_t parse_engine_args(int argc, char *argv[]) {
    engine_config_t config;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--verbose") config.verbose = true;
    }
    return config;
}

std::expected<engine_t, std::string>
create_engine(std::string_view title, int width, int height, engine_config_t const &config) {
    engine_t engine;
    engine.verbose = config.verbose;

    if (!SDL_Init(SDL_INIT_VIDEO)) return sdl_error("SDL_Init failed");
    engine.sdl_initialized = true;

    engine.window = SDL_CreateWindow(title.data(), width, height, 0);
    if (!engine.window) return sdl_error("SDL_CreateWindow failed");

    engine.gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    if (!engine.gpu_device) return sdl_error("SDL_CreateGPUDevice failed");

    if (!SDL_ClaimWindowForGPUDevice(engine.gpu_device, engine.window))
        return sdl_error("SDL_ClaimWindowForGPUDevice failed");

    if (engine.verbose) SDL_Log("GPU driver: %s", SDL_GetGPUDeviceDriver(engine.gpu_device));
    return engine;
}

bool poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) return false;
    }
    return true;
}

float tick(engine_t &engine) {
    Uint64 now = SDL_GetTicks();
    float dt = engine.last_tick == 0 ? 0.0f : (now - engine.last_tick) / 1000.0f;
    engine.last_tick = now;
    return dt;
}

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    std::function<void(SDL_GPUCommandBuffer *, SDL_GPURenderPass *)> draw
) {
    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!command_buffer) return sdl_error("SDL_AcquireGPUCommandBuffer failed");

    SDL_GPUTexture *swapchain_texture = nullptr;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(
            command_buffer, engine.window, &swapchain_texture, nullptr, nullptr
        )) {
        SDL_CancelGPUCommandBuffer(command_buffer);
        return sdl_error("SDL_WaitAndAcquireGPUSwapchainTexture failed");
    }

    if (swapchain_texture) {
        SDL_GPUColorTargetInfo target = {};
        target.texture = swapchain_texture;
        target.clear_color = clear_color;
        target.load_op = SDL_GPU_LOADOP_CLEAR;
        target.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass *render_pass =
            SDL_BeginGPURenderPass(command_buffer, &target, 1, nullptr);
        draw(command_buffer, render_pass);
        SDL_EndGPURenderPass(render_pass);
    }

    if (!SDL_SubmitGPUCommandBuffer(command_buffer))
        return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return {};
}

std::expected<gpu_shader_t, std::string> load_shader(
    engine_t const &engine, std::string_view spv_path, SDL_GPUShaderStage stage,
    Uint32 num_uniform_buffers, Uint32 num_samplers
) {
    std::ifstream file(spv_path.data(), std::ios::binary | std::ios::ate);
    if (!file) return std::unexpected(std::format("Cannot open shader: {}", spv_path));

    auto size = file.tellg();
    file.seekg(0);
    std::vector<Uint8> code(size);
    file.read(reinterpret_cast<char *>(code.data()), size);
    if (!file) return std::unexpected(std::format("Failed to read shader: {}", spv_path));

    SDL_GPUShaderCreateInfo info = {};
    info.code_size = code.size();
    info.code = code.data();
    info.entrypoint = "main";
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage = stage;
    info.num_uniform_buffers = num_uniform_buffers;
    info.num_samplers = num_samplers;

    SDL_GPUShader *shader = SDL_CreateGPUShader(engine.gpu_device, &info);
    if (!shader) return sdl_error("SDL_CreateGPUShader failed");
    return gpu_shader_t{engine.gpu_device, shader};
}

std::expected<gpu_buffer_t, std::string>
create_vertex_buffer(engine_t const &engine, void const *data, Uint32 size) {
    return create_buffer(engine, SDL_GPU_BUFFERUSAGE_VERTEX, data, size);
}

std::expected<gpu_buffer_t, std::string>
create_index_buffer(engine_t const &engine, void const *data, Uint32 size) {
    return create_buffer(engine, SDL_GPU_BUFFERUSAGE_INDEX, data, size);
}

std::expected<gpu_pipeline_t, std::string>
create_pipeline(engine_t const &engine, pipeline_desc_t const &desc) {
    auto vert = load_shader(engine, desc.vertex_shader, SDL_GPU_SHADERSTAGE_VERTEX,
                            desc.num_uniform_buffers);
    if (!vert) return std::unexpected(vert.error());

    auto frag = load_shader(engine, desc.fragment_shader, SDL_GPU_SHADERSTAGE_FRAGMENT,
                            0, desc.num_samplers);
    if (!frag) return std::unexpected(frag.error());

    SDL_GPUVertexBufferDescription buffer_desc = {};
    buffer_desc.slot       = 0;
    buffer_desc.pitch      = sizeof(vertex_t);
    buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attribute = {};
    attribute.location    = 0;
    attribute.buffer_slot = 0;
    attribute.format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribute.offset      = 0;

    SDL_GPUColorTargetDescription color_target = {};
    color_target.format = SDL_GetGPUSwapchainTextureFormat(engine.gpu_device, engine.window);

    SDL_GPUGraphicsPipelineCreateInfo info = {};
    info.vertex_shader   = vert->get();
    info.fragment_shader = frag->get();
    info.vertex_input_state.vertex_buffer_descriptions = &buffer_desc;
    info.vertex_input_state.num_vertex_buffers         = 1;
    info.vertex_input_state.vertex_attributes          = &attribute;
    info.vertex_input_state.num_vertex_attributes      = 1;
    info.primitive_type                                = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    info.target_info.color_target_descriptions         = &color_target;
    info.target_info.num_color_targets                 = 1;

    gpu_pipeline_t pipeline{engine.gpu_device,
                            SDL_CreateGPUGraphicsPipeline(engine.gpu_device, &info)};
    if (!pipeline) return sdl_error("SDL_CreateGPUGraphicsPipeline failed");
    return pipeline;
}
