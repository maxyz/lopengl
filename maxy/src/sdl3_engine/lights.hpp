#pragma once

#include <array>

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

// Vertex layout for pyramid geometry (positions-only, stride = 12 bytes).
// Used for spot-light direction indicators.
inline constexpr SDL_GPUVertexBufferDescription pyramid_buffer_descs[] = {
    {.slot = 0, .pitch = 3 * sizeof(float), .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX},
};

inline constexpr SDL_GPUVertexAttribute pyramid_vertex_attributes[] = {
    {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
};

// CPU-side light descriptions — human-readable values (world-space positions, angles in degrees).
// Converted to camera-relative GPU uniforms each frame in render().

constexpr int MAX_POS_LIGHTS  = 16;
constexpr int MAX_SPOT_LIGHTS = 8;

struct pos_light_state_t {
    glm::vec3 position;
    glm::vec3 ambient  = {0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse  = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct spot_light_state_t {
    glm::vec3 position;
    glm::vec3 direction     = {0.0f, 0.0f, -1.0f};
    glm::vec3 ambient       = {0.0f, 0.0f, 0.0f};
    glm::vec3 diffuse       = {0.5f, 0.5f, 0.5f};
    glm::vec3 specular      = {1.0f, 1.0f, 1.0f};
    float     inner_degrees = 25.0f;
    float     outer_degrees = 30.0f;
    float     constant = 1.0f, linear = 0.09f, quadratic = 0.032f;
};

struct flashlight_state_t {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     inner_degrees;
    float     outer_degrees;
    float     constant, linear, quadratic;
};

// GPU UBO array blocks — sent to the fragment shader each frame.
// Parameterised by count so fixed-size scenes (e.g. 4 lights) and dynamic scenes
// (e.g. MAX_POS_LIGHTS=16) can share the same struct name.
template <int N> struct pos_lights_block_t {
    std::array<positional_light_uniforms_t, N> lights;
};

template <int N> struct spot_lights_block_t {
    std::array<spot_light_uniforms_t, N> lights;
};
