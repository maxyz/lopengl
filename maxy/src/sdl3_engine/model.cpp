#include "model.hpp"

#include <format>
#include <functional>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "geometry.hpp"

namespace {

std::expected<gpu_geometry_t, std::string>
load_mesh_geometry(engine_t &engine, aiMesh const *mesh) {
    std::vector<pos_normal_uv_vertex_t> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        pos_normal_uv_vertex_t v;
        v.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        v.normal   = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        if (mesh->mTextureCoords[0])
            v.uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        vertices.push_back(v);
    }

    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        aiFace const &face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    Uint32 vertex_size = static_cast<Uint32>(vertices.size() * sizeof(pos_normal_uv_vertex_t));
    return create_geometry(
        engine, vertices.data(), vertex_size, std::span<uint32_t const>{indices}
    );
}

std::expected<int, std::string> cached_texture(
    engine_t &engine, gpu_model_t &model, std::unordered_map<std::string, int> &cache,
    std::string const &full_path
) {
    auto it = cache.find(full_path);
    if (it != cache.end()) return it->second;

    auto tex = load_texture(engine, full_path);
    if (!tex) return std::unexpected(tex.error());
    auto sampler = create_sampler(engine);
    if (!sampler) return std::unexpected(sampler.error());

    int index        = static_cast<int>(model.textures.size());
    cache[full_path] = index;
    model.textures.push_back(std::move(*tex));
    model.samplers.push_back(std::move(*sampler));
    return index;
}

std::expected<mesh_textures_t, std::string> load_mesh_textures(
    engine_t &engine, gpu_model_t &model, std::unordered_map<std::string, int> &cache,
    aiMaterial const *mat, std::string const &model_dir
) {
    mesh_textures_t result;

    auto load_slot = [&](aiTextureType type) -> std::expected<int, std::string> {
        aiString path;
        if (mat->GetTexture(type, 0, &path) != AI_SUCCESS) return -1;
        return cached_texture(engine, model, cache, model_dir + "/" + path.C_Str());
    };

    auto diffuse = load_slot(aiTextureType_DIFFUSE);
    if (!diffuse) return std::unexpected(diffuse.error());
    result.diffuse = *diffuse;

    auto specular = load_slot(aiTextureType_SPECULAR);
    if (!specular) return std::unexpected(specular.error());
    result.specular = *specular;

    return result;
}

} // namespace

std::expected<gpu_model_t, std::string> load_model(engine_t &engine, std::string_view path) {
    Assimp::Importer importer;
    aiScene const   *ai_scene = importer.ReadFile(
        path.data(), static_cast<unsigned>(aiProcess_Triangulate | aiProcess_FlipUVs)
    );
    if (!ai_scene || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !ai_scene->mRootNode)
        return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));

    std::string model_dir{path.substr(0, path.find_last_of("/\\"))};

    gpu_model_t                          model;
    std::unordered_map<std::string, int> cache;

    std::function<std::expected<void, std::string>(aiNode const *)> process_node;
    process_node = [&](aiNode const *node) -> std::expected<void, std::string> {
        for (unsigned i = 0; i < node->mNumMeshes; ++i) {
            aiMesh const     *mesh = ai_scene->mMeshes[node->mMeshes[i]];
            aiMaterial const *mat  = ai_scene->mMaterials[mesh->mMaterialIndex];

            auto geom = load_mesh_geometry(engine, mesh);
            if (!geom) return std::unexpected(geom.error());

            auto tex = load_mesh_textures(engine, model, cache, mat, model_dir);
            if (!tex) return std::unexpected(tex.error());

            model.meshes.push_back({std::move(*geom), *tex});
        }
        for (unsigned i = 0; i < node->mNumChildren; ++i) {
            if (auto r = process_node(node->mChildren[i]); !r) return std::unexpected(r.error());
        }
        return {};
    };

    if (auto r = process_node(ai_scene->mRootNode); !r) return std::unexpected(r.error());

    return model;
}

void draw_model(
    gpu_model_t const &model, std::initializer_list<texture_slot_t> sampler_slots,
    SDL_GPURenderPass *pass
) {
    for (auto const &mesh : model.meshes) {
        std::vector<SDL_GPUTextureSamplerBinding> bindings;
        bindings.reserve(sampler_slots.size());
        bool complete = true;
        for (texture_slot_t slot : sampler_slots) {
            int idx = -1;
            switch (slot) {
            case texture_slot_t::diffuse:
                idx = mesh.textures.diffuse;
                break;
            case texture_slot_t::specular:
                idx = mesh.textures.specular;
                break;
            }
            if (idx < 0) {
                complete = false;
                break;
            }
            bindings.push_back({model.textures[idx].get(), model.samplers[idx].get()});
        }
        if (!complete) continue;

        SDL_GPUBufferBinding vbinding{mesh.geometry.vertex_buffer.get(), 0};
        SDL_BindGPUVertexBuffers(pass, 0, &vbinding, 1);
        SDL_GPUBufferBinding ibinding{mesh.geometry.index_buffer.get(), 0};
        SDL_BindGPUIndexBuffer(pass, &ibinding, mesh.geometry.index_element_size);
        SDL_BindGPUFragmentSamplers(pass, 0, bindings.data(), static_cast<Uint32>(bindings.size()));
        SDL_DrawGPUIndexedPrimitives(pass, mesh.geometry.index_count, 1, 0, 0, 0);
    }
}

void draw_model(
    gpu_pipeline_t const &pipeline, gpu_model_t const &model,
    std::initializer_list<texture_slot_t> sampler_slots, SDL_GPURenderPass *pass
) {
    SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
    draw_model(model, sampler_slots, pass);
}
