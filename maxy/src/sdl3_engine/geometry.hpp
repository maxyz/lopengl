#pragma once
#include <array>
#include <cstdint>
#include <numbers>

#include <glm/glm.hpp>

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

struct pos_uv_vertex_t {
    vertex_t position; // location 0
    uv_t     uv;       // location 1
};

struct pos_normal_uv_vertex_t {
    vertex_t position; // location 0
    vertex_t normal;   // location 1
    uv_t     uv;       // location 2
};

// Vertical unit square in the XY plane, centered at the origin. Normal points +Z.
// Use for billboard / vegetation quads. Back-face culling is off by default in SDL3 GPU,
// so the quad is visible from both sides without duplicating vertices.
inline constexpr std::array<pos_normal_uv_vertex_t, 6> vertical_quad_vertices = {{
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
}};

// Tiled floor plane: 10x10 quad at y=0, normal up, UV tiles 2x in each direction.
// Use with a REPEAT sampler (the engine default) for a seamless tile effect.
inline constexpr std::array<pos_normal_uv_vertex_t, 6> floor_plane_vertices = {{
    {{5.f, 0.f, 5.f}, {0.f, 1.f, 0.f}, {2.f, 0.f}},
    {{-5.f, 0.f, -5.f}, {0.f, 1.f, 0.f}, {0.f, 2.f}},
    {{-5.f, 0.f, 5.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
    {{5.f, 0.f, 5.f}, {0.f, 1.f, 0.f}, {2.f, 0.f}},
    {{5.f, 0.f, -5.f}, {0.f, 1.f, 0.f}, {2.f, 2.f}},
    {{-5.f, 0.f, -5.f}, {0.f, 1.f, 0.f}, {0.f, 2.f}},
}};

// 10 world-space positions used across tutorial chapters 9 and 10.
inline constexpr std::array<glm::vec3, 10> example_cube_positions = {{
    {0.0f, 0.0f, 0.0f},
    {2.0f, 5.0f, -15.0f},
    {-1.5f, -2.2f, -2.5f},
    {-3.8f, -2.0f, -12.3f},
    {2.4f, -0.4f, -3.5f},
    {-1.7f, 3.0f, -7.5f},
    {1.3f, -2.0f, -2.5f},
    {1.5f, 2.0f, -2.5f},
    {1.5f, 0.2f, -1.5f},
    {-1.3f, 1.0f, -1.5f},
}};

// Unit cube: 36 vertices (6 faces x 2 triangles x 3), no index buffer.
inline constexpr std::array<pos_uv_vertex_t, 36> unit_cube = {{
    // back
    {{-.5f, -.5f, -.5f}, {.0f, .0f}},
    {{.5f, -.5f, -.5f}, {1.f, .0f}},
    {{.5f, .5f, -.5f}, {1.f, 1.f}},
    {{.5f, .5f, -.5f}, {1.f, 1.f}},
    {{-.5f, .5f, -.5f}, {.0f, 1.f}},
    {{-.5f, -.5f, -.5f}, {.0f, .0f}},
    // front
    {{.5f, -.5f, .5f}, {.0f, .0f}},
    {{-.5f, -.5f, .5f}, {1.f, .0f}},
    {{-.5f, .5f, .5f}, {1.f, 1.f}},
    {{-.5f, .5f, .5f}, {1.f, 1.f}},
    {{.5f, .5f, .5f}, {.0f, 1.f}},
    {{.5f, -.5f, .5f}, {.0f, .0f}},
    // left
    {{-.5f, -.5f, .5f}, {.0f, .0f}},
    {{-.5f, -.5f, -.5f}, {1.f, .0f}},
    {{-.5f, .5f, -.5f}, {1.f, 1.f}},
    {{-.5f, .5f, -.5f}, {1.f, 1.f}},
    {{-.5f, .5f, .5f}, {.0f, 1.f}},
    {{-.5f, -.5f, .5f}, {.0f, .0f}},
    // right
    {{.5f, -.5f, -.5f}, {.0f, .0f}},
    {{.5f, -.5f, .5f}, {1.f, .0f}},
    {{.5f, .5f, .5f}, {1.f, 1.f}},
    {{.5f, .5f, .5f}, {1.f, 1.f}},
    {{.5f, .5f, -.5f}, {.0f, 1.f}},
    {{.5f, -.5f, -.5f}, {.0f, .0f}},
    // bottom
    {{-.5f, -.5f, -.5f}, {.0f, .0f}},
    {{.5f, -.5f, -.5f}, {1.f, .0f}},
    {{.5f, -.5f, .5f}, {1.f, 1.f}},
    {{.5f, -.5f, .5f}, {1.f, 1.f}},
    {{-.5f, -.5f, .5f}, {.0f, 1.f}},
    {{-.5f, -.5f, -.5f}, {.0f, .0f}},
    // top
    {{-.5f, .5f, .5f}, {.0f, .0f}},
    {{.5f, .5f, .5f}, {1.f, .0f}},
    {{.5f, .5f, -.5f}, {1.f, 1.f}},
    {{.5f, .5f, -.5f}, {1.f, 1.f}},
    {{-.5f, .5f, -.5f}, {.0f, 1.f}},
    {{-.5f, .5f, .5f}, {.0f, .0f}},
}};

// Unit cube with per-face outward normals: 36 vertices, pos+normal+UV, no index buffer.
inline constexpr std::array<pos_normal_uv_vertex_t, 36> unit_cube_with_normals = {{
    // back  (normal 0, 0, -1)
    {{-.5f, -.5f, -.5f}, {.0f, .0f, -1.f}, {.0f, .0f}},
    {{.5f, .5f, -.5f}, {.0f, .0f, -1.f}, {1.f, 1.f}},
    {{.5f, -.5f, -.5f}, {.0f, .0f, -1.f}, {1.f, .0f}},
    {{.5f, .5f, -.5f}, {.0f, .0f, -1.f}, {1.f, 1.f}},
    {{-.5f, -.5f, -.5f}, {.0f, .0f, -1.f}, {.0f, .0f}},
    {{-.5f, .5f, -.5f}, {.0f, .0f, -1.f}, {.0f, 1.f}},
    // front  (normal 0, 0, 1)
    {{.5f, -.5f, .5f}, {.0f, .0f, 1.f}, {.0f, .0f}},
    {{-.5f, .5f, .5f}, {.0f, .0f, 1.f}, {1.f, 1.f}},
    {{-.5f, -.5f, .5f}, {.0f, .0f, 1.f}, {1.f, .0f}},
    {{-.5f, .5f, .5f}, {.0f, .0f, 1.f}, {1.f, 1.f}},
    {{.5f, -.5f, .5f}, {.0f, .0f, 1.f}, {.0f, .0f}},
    {{.5f, .5f, .5f}, {.0f, .0f, 1.f}, {.0f, 1.f}},
    // left  (normal -1, 0, 0)
    {{-.5f, -.5f, .5f}, {-1.f, .0f, .0f}, {.0f, .0f}},
    {{-.5f, .5f, -.5f}, {-1.f, .0f, .0f}, {1.f, 1.f}},
    {{-.5f, -.5f, -.5f}, {-1.f, .0f, .0f}, {1.f, .0f}},
    {{-.5f, .5f, -.5f}, {-1.f, .0f, .0f}, {1.f, 1.f}},
    {{-.5f, -.5f, .5f}, {-1.f, .0f, .0f}, {.0f, .0f}},
    {{-.5f, .5f, .5f}, {-1.f, .0f, .0f}, {.0f, 1.f}},
    // right  (normal 1, 0, 0)
    {{.5f, -.5f, -.5f}, {1.f, .0f, .0f}, {.0f, .0f}},
    {{.5f, .5f, .5f}, {1.f, .0f, .0f}, {1.f, 1.f}},
    {{.5f, -.5f, .5f}, {1.f, .0f, .0f}, {1.f, .0f}},
    {{.5f, .5f, .5f}, {1.f, .0f, .0f}, {1.f, 1.f}},
    {{.5f, -.5f, -.5f}, {1.f, .0f, .0f}, {.0f, .0f}},
    {{.5f, .5f, -.5f}, {1.f, .0f, .0f}, {.0f, 1.f}},
    // bottom  (normal 0, -1, 0)
    {{-.5f, -.5f, -.5f}, {.0f, -1.f, .0f}, {.0f, .0f}},
    {{.5f, -.5f, -.5f}, {.0f, -1.f, .0f}, {1.f, .0f}},
    {{.5f, -.5f, .5f}, {.0f, -1.f, .0f}, {1.f, 1.f}},
    {{.5f, -.5f, .5f}, {.0f, -1.f, .0f}, {1.f, 1.f}},
    {{-.5f, -.5f, .5f}, {.0f, -1.f, .0f}, {.0f, 1.f}},
    {{-.5f, -.5f, -.5f}, {.0f, -1.f, .0f}, {.0f, .0f}},
    // top  (normal 0, 1, 0)
    {{-.5f, .5f, .5f}, {.0f, 1.f, .0f}, {.0f, .0f}},
    {{.5f, .5f, .5f}, {.0f, 1.f, .0f}, {1.f, .0f}},
    {{.5f, .5f, -.5f}, {.0f, 1.f, .0f}, {1.f, 1.f}},
    {{.5f, .5f, -.5f}, {.0f, 1.f, .0f}, {1.f, 1.f}},
    {{-.5f, .5f, -.5f}, {.0f, 1.f, .0f}, {.0f, 1.f}},
    {{-.5f, .5f, .5f}, {.0f, 1.f, .0f}, {.0f, .0f}},
}};

// Pyramid indicator: apex at origin, base at z = -1.
// Used for spot-light direction markers; pair with pyramid_buffer_descs / pyramid_vertex_attributes
// from lights.hpp.
inline constexpr float pyramid_vertices[] = {
    0.0f,  0.0f,  0.0f,  // apex
    -0.5f, 0.5f,  -1.0f, // base TL
    0.5f,  0.5f,  -1.0f, // base TR
    0.5f,  -0.5f, -1.0f, // base BR
    -0.5f, -0.5f, -1.0f, // base BL
};

inline constexpr uint16_t pyramid_indices[] = {
    0, 2, 1,                            // sides
    0, 3, 2, 0, 4, 3, 0, 1, 4, 1, 2, 3, // base
    1, 3, 4,
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
