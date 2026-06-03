#pragma once
#include <expected>
#include <functional>
#include <string>
#include <string_view>

#include <SDL3/SDL.h>

struct engine_t {
    SDL_Window *window = nullptr;
    SDL_GPUDevice *gpu_device = nullptr;
    bool sdl_initialized = false;

    engine_t() = default;
    engine_t(engine_t const &) = delete;
    engine_t &operator=(engine_t const &) = delete;
    engine_t(engine_t &&) noexcept;
    engine_t &operator=(engine_t &&) noexcept;
    ~engine_t();
};

std::expected<engine_t, std::string>
create_engine(std::string_view title, int width, int height);

// Returns false when the application should quit (window closed).
// Games handle their own exit conditions (escape, menus, etc.) separately.
bool poll_events();

std::expected<void, std::string> render_frame(
    engine_t const &engine, SDL_FColor clear_color,
    std::function<void(SDL_GPURenderPass *)> draw
);
