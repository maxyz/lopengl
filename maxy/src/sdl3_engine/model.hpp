#pragma once
#include <expected>
#include <initializer_list>
#include <string_view>
#include <vector>

#include "engine.hpp"

// Which texture type to bind at a given fragment sampler slot.
// Pass as an ordered list to draw_model(); the list length must match
// the pipeline's fragment_samplers count.
enum class texture_slot_t { diffuse, specular };

// Typed texture indices into a gpu_model_t's flat texture list.
// -1 means the mesh has no texture of that type.
struct mesh_textures_t {
    int diffuse  = -1;
    int specular = -1;
};

// Geometry and texture references for one mesh inside a loaded model.
struct model_mesh_t {
    gpu_geometry_t  geometry;
    mesh_textures_t textures;
};

// Owns all unique textures for a loaded model. Meshes reference textures by
// index so the same file is uploaded to the GPU only once.
struct gpu_model_t {
    std::vector<gpu_texture_t> textures;
    std::vector<gpu_sampler_t> samplers;
    std::vector<model_mesh_t>  meshes;
};

// Loads a model from disk via Assimp (triangulates and flips UVs).
// Textures are deduplicated: the same file is uploaded at most once.
// Diffuse and specular texture types are populated when present.
std::expected<gpu_model_t, std::string> load_model(engine_t &engine, std::string_view path);

// Draw all meshes that have every requested texture slot.
// Caller must have already bound the pipeline and pushed uniforms.
void draw_model(
    gpu_model_t const &model, std::initializer_list<texture_slot_t> sampler_slots,
    SDL_GPURenderPass *pass
);

// Convenience overload: binds the pipeline then draws.
void draw_model(
    gpu_pipeline_t const &pipeline, gpu_model_t const &model,
    std::initializer_list<texture_slot_t> sampler_slots, SDL_GPURenderPass *pass
);
