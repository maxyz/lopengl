#include "engine.hpp"

#include <format>
#include <utility>

namespace {

std::unexpected<std::string> sdl_error(std::string prefix) {
    return std::unexpected(std::format("{}: {}", prefix, SDL_GetError()));
}

} // namespace

engine_t::engine_t(engine_t &&other) noexcept
    : window(std::exchange(other.window, nullptr)),
      gpu_device(std::exchange(other.gpu_device, nullptr)),
      sdl_initialized(std::exchange(other.sdl_initialized, false)) {}

engine_t &engine_t::operator=(engine_t &&other) noexcept {
    if (this != &other) {
        this->~engine_t();
        window = std::exchange(other.window, nullptr);
        gpu_device = std::exchange(other.gpu_device, nullptr);
        sdl_initialized = std::exchange(other.sdl_initialized, false);
    }
    return *this;
}

engine_t::~engine_t() {
    if (gpu_device && window)
        SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    if (gpu_device)
        SDL_DestroyGPUDevice(gpu_device);
    if (window)
        SDL_DestroyWindow(window);
    if (sdl_initialized)
        SDL_Quit();
}

std::expected<engine_t, std::string>
create_engine(std::string_view title, int width, int height) {
    engine_t engine;

    if (!SDL_Init(SDL_INIT_VIDEO))
        return sdl_error("SDL_Init failed");
    engine.sdl_initialized = true;

    engine.window = SDL_CreateWindow(title.data(), width, height, 0);
    if (!engine.window)
        return sdl_error("SDL_CreateWindow failed");

    engine.gpu_device =
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
    if (!engine.gpu_device)
        return sdl_error("SDL_CreateGPUDevice failed");

    if (!SDL_ClaimWindowForGPUDevice(engine.gpu_device, engine.window))
        return sdl_error("SDL_ClaimWindowForGPUDevice failed");

    SDL_Log("GPU driver: %s", SDL_GetGPUDeviceDriver(engine.gpu_device));
    return engine;
}

bool poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;
    }
    return true;
}

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    std::function<void(SDL_GPURenderPass *)> draw
) {
    SDL_GPUCommandBuffer *command_buffer =
        SDL_AcquireGPUCommandBuffer(engine.gpu_device);
    if (!command_buffer)
        return sdl_error("SDL_AcquireGPUCommandBuffer failed");

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
        draw(render_pass);
        SDL_EndGPURenderPass(render_pass);
    }

    if (!SDL_SubmitGPUCommandBuffer(command_buffer))
        return sdl_error("SDL_SubmitGPUCommandBuffer failed");

    return {};
}
