#pragma once

#include <array>
#include <cstddef>
#include <glm/glm.hpp>

struct cube_vertex_t {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texcoord;
};

// Unit cube: 36 vertices (6 faces × 2 triangles × 3), no index buffer.
// Stride = sizeof(cube_vertex_t). Use offsetof() for attrib pointers.
inline const std::array<cube_vertex_t, 36> cube_vertices = {{
    {{-.5f, -.5f, -.5f}, { .0f,  .0f, -1.f}, {.0f, .0f}}, // back
    {{ .5f, -.5f, -.5f}, { .0f,  .0f, -1.f}, {1.f, .0f}},
    {{ .5f,  .5f, -.5f}, { .0f,  .0f, -1.f}, {1.f, 1.f}},
    {{ .5f,  .5f, -.5f}, { .0f,  .0f, -1.f}, {1.f, 1.f}},
    {{-.5f,  .5f, -.5f}, { .0f,  .0f, -1.f}, {.0f, 1.f}},
    {{-.5f, -.5f, -.5f}, { .0f,  .0f, -1.f}, {.0f, .0f}},

    {{ .5f, -.5f,  .5f}, { .0f,  .0f,  1.f}, {.0f, .0f}}, // front
    {{-.5f, -.5f,  .5f}, { .0f,  .0f,  1.f}, {1.f, .0f}},
    {{-.5f,  .5f,  .5f}, { .0f,  .0f,  1.f}, {1.f, 1.f}},
    {{-.5f,  .5f,  .5f}, { .0f,  .0f,  1.f}, {1.f, 1.f}},
    {{ .5f,  .5f,  .5f}, { .0f,  .0f,  1.f}, {.0f, 1.f}},
    {{ .5f, -.5f,  .5f}, { .0f,  .0f,  1.f}, {.0f, .0f}},

    {{-.5f, -.5f,  .5f}, {-1.f,  .0f,  .0f}, {.0f, .0f}}, // left
    {{-.5f, -.5f, -.5f}, {-1.f,  .0f,  .0f}, {1.f, .0f}},
    {{-.5f,  .5f, -.5f}, {-1.f,  .0f,  .0f}, {1.f, 1.f}},
    {{-.5f,  .5f, -.5f}, {-1.f,  .0f,  .0f}, {1.f, 1.f}},
    {{-.5f,  .5f,  .5f}, {-1.f,  .0f,  .0f}, {.0f, 1.f}},
    {{-.5f, -.5f,  .5f}, {-1.f,  .0f,  .0f}, {.0f, .0f}},

    {{ .5f, -.5f, -.5f}, { 1.f,  .0f,  .0f}, {.0f, .0f}}, // right
    {{ .5f, -.5f,  .5f}, { 1.f,  .0f,  .0f}, {1.f, .0f}},
    {{ .5f,  .5f,  .5f}, { 1.f,  .0f,  .0f}, {1.f, 1.f}},
    {{ .5f,  .5f,  .5f}, { 1.f,  .0f,  .0f}, {1.f, 1.f}},
    {{ .5f,  .5f, -.5f}, { 1.f,  .0f,  .0f}, {.0f, 1.f}},
    {{ .5f, -.5f, -.5f}, { 1.f,  .0f,  .0f}, {.0f, .0f}},

    {{-.5f, -.5f, -.5f}, { .0f, -1.f,  .0f}, {.0f, .0f}}, // bottom
    {{ .5f, -.5f, -.5f}, { .0f, -1.f,  .0f}, {1.f, .0f}},
    {{ .5f, -.5f,  .5f}, { .0f, -1.f,  .0f}, {1.f, 1.f}},
    {{ .5f, -.5f,  .5f}, { .0f, -1.f,  .0f}, {1.f, 1.f}},
    {{-.5f, -.5f,  .5f}, { .0f, -1.f,  .0f}, {.0f, 1.f}},
    {{-.5f, -.5f, -.5f}, { .0f, -1.f,  .0f}, {.0f, .0f}},

    {{-.5f,  .5f,  .5f}, { .0f,  1.f,  .0f}, {.0f, .0f}}, // top
    {{ .5f,  .5f,  .5f}, { .0f,  1.f,  .0f}, {1.f, .0f}},
    {{ .5f,  .5f, -.5f}, { .0f,  1.f,  .0f}, {1.f, 1.f}},
    {{ .5f,  .5f, -.5f}, { .0f,  1.f,  .0f}, {1.f, 1.f}},
    {{-.5f,  .5f, -.5f}, { .0f,  1.f,  .0f}, {.0f, 1.f}},
    {{-.5f,  .5f,  .5f}, { .0f,  1.f,  .0f}, {.0f, .0f}},
}};

// Example positions for scattering 10 cubes around a scene.
inline const glm::vec3 example_cube_positions[] = {
    {0.0f,   0.0f,   0.0f},
    {2.0f,   5.0f, -15.0f},
    {-1.5f, -2.2f,  -2.5f},
    {-3.8f, -2.0f, -12.3f},
    {2.4f,  -0.4f,  -3.5f},
    {-1.7f,  3.0f,  -7.5f},
    {1.3f,  -2.0f,  -2.5f},
    {1.5f,   2.0f,  -2.5f},
    {1.5f,   0.2f,  -1.5f},
    {-1.3f,  1.0f,  -1.5f},
};

// Pyramid: apex at origin, square base at z = -1, positions only.
// Stride = 3 * sizeof(float).
inline constexpr float pyramid_vertices[] = {
    0.f,   0.f,  0.f,  // apex
    -.5f,  .5f, -1.f,  // base
     .5f,  .5f, -1.f,
     .5f, -.5f, -1.f,
    -.5f, -.5f, -1.f,
};

inline constexpr unsigned int pyramid_indices[] = {
    0, 1, 2, // side 1
    0, 2, 3, // side 2
    0, 3, 4, // side 3
    0, 4, 1, // side 4
    1, 2, 3, // base 1
    1, 3, 4, // base 2
};
