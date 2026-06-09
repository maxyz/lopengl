#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include "geometry.hpp"

struct light_uniforms_t {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 position; // camera-relative world space; updated each frame
};

struct directional_light_uniforms_t {
    glm::vec4
        direction; // world-space direction toward scene; negated in shader to get toward-light
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

// Attenuation: 1 / (constant + linear*d + quadratic*d^2).
// Coefficients (1.0, 0.09, 0.032) give ~20-unit range; see learnopengl.com/Lighting/Light-casters.
struct positional_light_uniforms_t {
    glm::vec4 position; // camera-relative world space; updated each frame
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float     constant;
    float     linear;
    float     quadratic;
    float     pad; // struct size must be a multiple of 16 (std140 alignment)
};

// Spotlight: positional light restricted to a cone with soft edges.
// cutoff/outer_cutoff are cosines of angles (precomputed on CPU to avoid acos per fragment).
// 5 vec4s (80 B) + 5 floats (20 B) + 3 pad floats (12 B) = 112 B (multiple of 16, std140).
struct spot_light_uniforms_t {
    glm::vec4 position;  // camera-relative world space
    glm::vec4 direction; // world-space cone axis direction
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float     cutoff;       // cos(inner_angle)
    float     outer_cutoff; // cos(outer_angle)
    float     constant;
    float     linear;
    float     quadratic;
    float     pad[3];
};

// Camera-attached spotlight. Position is implicitly vec3(0) in camera-relative world space.
// Direction is camera.front(), updated each frame.
// 4 vec4s (64 B) + 5 floats (20 B) + 3 pad floats (12 B) = 96 B (std140 multiple of 16).
struct flashlight_uniforms_t {
    glm::vec4 direction; // camera.front() in camera-relative world space; updated each frame
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float     cutoff;       // cos(inner_angle)
    float     outer_cutoff; // cos(outer_angle)
    float     constant;
    float     linear;
    float     quadratic;
    float     pad[3];
};

// Position-only vertex attribute for the light-indicator cube pipeline.
// Reuses the same VBO as the scene cube; only the position channel is read.
inline constexpr SDL_GPUVertexAttribute light_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, position))},
};
