#pragma once
#include <array>
#include <numbers>

struct vertex_t {
    float x, y, z;
};

// Returns an equilateral triangle inscribed in a circle of the given
// circumradius, top vertex pointing up.
constexpr std::array<vertex_t, 3> make_equilateral_triangle(float circumradius) {
    float half_base = circumradius * std::numbers::sqrt3_v<float> / 2.0f;
    float top_y     = circumradius;
    float bottom_y  = -circumradius / 2.0f;
    return {{
        { 0.0f,      top_y,    0.0f},
        {-half_base, bottom_y, 0.0f},
        { half_base, bottom_y, 0.0f},
    }};
}
