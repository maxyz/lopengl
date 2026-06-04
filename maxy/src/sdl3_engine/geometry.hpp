#pragma once
#include <array>
#include <cstdint>
#include <numbers>

struct vertex_t {
    float x, y, z;
};

struct color_t {
    float r, g, b;
};

struct colored_vertex_t {
    vertex_t position; // location 0
    color_t  color;    // location 1
};

struct uv_t {
    float u, v;
};

struct textured_vertex_t {
    vertex_t position; // location 0
    color_t  color;    // location 1
    uv_t     uv;       // location 2
};

// Returns an equilateral triangle inscribed in a circle of the given
// circumradius, top vertex pointing up.
constexpr std::array<vertex_t, 3> make_equilateral_triangle(float circumradius) {
    float half_base = circumradius * std::numbers::sqrt3_v<float> / 2.0f;
    float top_y     = circumradius;
    float bottom_y  = -circumradius / 2.0f;
    return {{
        {0.0f, top_y, 0.0f},
        {-half_base, bottom_y, 0.0f},
        {half_base, bottom_y, 0.0f},
    }};
}

struct rhombus_t {
    std::array<vertex_t, 4> vertices;
    std::array<uint16_t, 6> indices;
};

// Returns two equilateral triangles sharing an edge, forming a rhombus.
// The shared edge is horizontal; top and bottom vertices are the apices.
// Vertices are wound CCW. Indices are 16-bit.
constexpr rhombus_t make_equilateral_rhombus(float side_length) {
    float half    = side_length / 2.0f;
    float apothem = side_length * std::numbers::sqrt3_v<float> / 2.0f;
    return {
        {{
            {-half, 0.0f, 0.0f},    // 0: left
            {half, 0.0f, 0.0f},     // 1: right
            {0.0f, apothem, 0.0f},  // 2: top
            {0.0f, -apothem, 0.0f}, // 3: bottom
        }},
        {{0, 1, 2,  // top triangle
          0, 3, 1}} // bottom triangle
    };
}
