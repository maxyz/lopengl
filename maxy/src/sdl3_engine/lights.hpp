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

// Position-only vertex attribute for the light-indicator cube pipeline.
// Reuses the same VBO as the scene cube; only the position channel is read.
inline constexpr SDL_GPUVertexAttribute light_vertex_attributes[] = {
    {.location    = 0,
     .buffer_slot = 0,
     .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
     .offset      = static_cast<Uint32>(offsetof(pos_normal_uv_vertex_t, position))},
};
