#include <print>

#include <SDL3/SDL.h>

#include "engine.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

struct color_state_t {
    float r     = 0.2f;
    float g     = 0.3f;
    float b     = 0.3f;
    float delta = 0.06f; // units per second (matches original ~0.001/frame at 60fps)
};

void advance_color(color_state_t &color, float dt) {
    float step = color.delta * dt;
    color.r += step;
    if (color.r > 1.0f) color.r -= 1.0f;
    color.g += step;
    if (color.g > 1.0f) color.g -= 1.0f;
    color.b += step;
    if (color.b > 1.0f) color.b -= 1.0f;
}

int main(int argc, char *argv[]) {
    auto config      = parse_engine_args(argc, argv);
    auto init_result = create_engine("LOpenGL SDL3", WINDOW_WIDTH, WINDOW_HEIGHT, config);
    if (!init_result) {
        std::println(stderr, "Initialization failed: {}", init_result.error());
        return 1;
    }

    engine_t     &engine = *init_result;
    color_state_t color;

    while (poll_events()) {
        float       dt   = tick(engine);
        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE]) break;

        SDL_FColor clear_color = {color.r, color.g, color.b, 1.0f};
        if (auto frame = render_frame(
                engine, clear_color, [](SDL_GPUCommandBuffer *, SDL_GPURenderPass *) {}
            );
            !frame) {
            std::println(stderr, "Render error: {}", frame.error());
            break;
        }

        advance_color(color, dt);
    }

    return 0;
}
