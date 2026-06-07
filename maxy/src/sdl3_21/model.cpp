#include <print>
#include <unordered_map>

#include <SDL3/SDL.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "engine.hpp"
#include "geometry.hpp"

constexpr int        WINDOW_WIDTH     = 1024;
constexpr int        WINDOW_HEIGHT    = 768;
constexpr SDL_FColor BACKGROUND_COLOR = {0.52f, 0.39f, 0.27f, 1.0f}; // desert sand

// One mesh's draw data: indices into the model's shared texture list.
struct model_mesh_t {
    gpu_geometry_t geometry;
    int            diffuse_index = -1; // -1 means no diffuse texture
};

// Owns all unique textures for a model. Each mesh references textures by index,
// so textures loaded from the same path are shared rather than duplicated.
struct gpu_model_t {
    std::vector<gpu_texture_t> textures;
    std::vector<gpu_sampler_t> samplers;
    std::vector<model_mesh_t>  meshes;
};

struct scene_t {
    gpu_pipeline_t pipeline;
    gpu_model_t    model;
    camera_t       camera;
    float          m_aspect_ratio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

    bool update(input_t const &in);
    void render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass);
};

bool scene_t::update(input_t const &in) {
    m_aspect_ratio = in.aspect_ratio;
    if (in.keys[SDL_SCANCODE_ESCAPE]) return false;
    camera.update(in);

    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::LabelText(
        "Camera", "(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z
    );
    ImGui::LabelText("Mode", "%s", camera.ui_mode() ? "UI (` to fly)" : "Fly (` for UI)");
    ImGui::LabelText("Meshes", "%zu", model.meshes.size());
    ImGui::LabelText("Textures", "%zu", model.textures.size());
    ImGui::End();

    return true;
}

void scene_t::render(SDL_GPUCommandBuffer *cmd, SDL_GPURenderPass *pass) {
    glm::mat4 model_mat  = glm::mat4(1.0f);
    glm::mat4 view       = camera.view_matrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), m_aspect_ratio, 0.1f, 100.0f);

    // Pipeline and MVP uniforms are constant across all meshes: bind once.
    SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
    push_vertex_uniform(cmd, 0, model_mat);
    push_vertex_uniform(cmd, 1, view);
    push_vertex_uniform(cmd, 2, projection);

    for (auto const &mesh : model.meshes) {
        if (mesh.diffuse_index < 0) continue;

        SDL_GPUBufferBinding vbinding{mesh.geometry.vertex_buffer.get(), 0};
        SDL_BindGPUVertexBuffers(pass, 0, &vbinding, 1);
        SDL_GPUBufferBinding ibinding{mesh.geometry.index_buffer.get(), 0};
        SDL_BindGPUIndexBuffer(pass, &ibinding, mesh.geometry.index_element_size);

        SDL_GPUTextureSamplerBinding tbinding{
            model.textures[mesh.diffuse_index].get(), model.samplers[mesh.diffuse_index].get()
        };
        SDL_BindGPUFragmentSamplers(pass, 0, &tbinding, 1);
        SDL_DrawGPUIndexedPrimitives(pass, mesh.geometry.index_count, 1, 0, 0, 0);
    }
}

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

std::expected<gpu_model_t, std::string>
load_model(engine_t &engine, aiScene const *ai_scene, std::string const &model_dir) {
    gpu_model_t                          model;
    std::unordered_map<std::string, int> texture_cache; // path -> index into model.textures

    auto load_diffuse = [&](aiMaterial const *mat) -> std::expected<int, std::string> {
        aiString path;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) != AI_SUCCESS) return -1;

        std::string full_path = model_dir + "/" + path.C_Str();
        auto        cached    = texture_cache.find(full_path);
        if (cached != texture_cache.end()) return cached->second;

        auto tex = load_texture(engine, full_path);
        if (!tex) return std::unexpected(tex.error());
        auto sampler = create_sampler(engine);
        if (!sampler) return std::unexpected(sampler.error());

        int index                = static_cast<int>(model.textures.size());
        texture_cache[full_path] = index;
        model.textures.push_back(std::move(*tex));
        model.samplers.push_back(std::move(*sampler));
        return index;
    };

    std::function<std::expected<void, std::string>(aiNode const *)> process_node;
    process_node = [&](aiNode const *node) -> std::expected<void, std::string> {
        for (unsigned i = 0; i < node->mNumMeshes; ++i) {
            aiMesh const     *mesh = ai_scene->mMeshes[node->mMeshes[i]];
            aiMaterial const *mat  = ai_scene->mMaterials[mesh->mMaterialIndex];

            auto geom = load_mesh_geometry(engine, mesh);
            if (!geom) return std::unexpected(geom.error());

            auto diffuse_idx = load_diffuse(mat);
            if (!diffuse_idx) return std::unexpected(diffuse_idx.error());

            model.meshes.push_back({std::move(*geom), *diffuse_idx});
        }
        for (unsigned i = 0; i < node->mNumChildren; ++i) {
            if (auto r = process_node(node->mChildren[i]); !r) return std::unexpected(r.error());
        }
        return {};
    };

    if (auto r = process_node(ai_scene->mRootNode); !r) return std::unexpected(r.error());

    return model;
}

} // namespace

std::expected<scene_t, std::string> create_scene(engine_t &engine) {
    scene_t scene;
    scene.camera = camera_t(engine.window);

    if (auto r = init_imgui(engine); !r) return std::unexpected(r.error());
    ImGuiIO &io    = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromFileTTF(
        (std::string(ASSETS_PATH) + "fonts/NotoSans-Regular.ttf").c_str(), 20.0f
    );

    auto pipe = create_pipeline(
        engine, {
                    .vertex_shader            = "shaders/sdl3_21/model.vert.spv",
                    .fragment_shader          = "shaders/sdl3_21/model.frag.spv",
                    .vertex_uniform_buffers   = 3,
                    .fragment_uniform_buffers = 0,
                    .fragment_samplers        = 1,
                    .vertex_buffer_descs      = pos_normal_uv_buffer_descs,
                    .vertex_attributes        = pos_normal_uv_vertex_attributes,
                    .enable_depth_test        = true,
                }
    );
    if (!pipe) return std::unexpected(pipe.error());
    scene.pipeline = std::move(*pipe);

    std::string model_path = std::string(ASSETS_PATH) + "objects/backpack/backpack.obj";
    std::string model_dir  = model_path.substr(0, model_path.find_last_of("/\\"));

    Assimp::Importer importer;
    aiScene const   *ai_scene = importer.ReadFile(
        model_path, static_cast<unsigned>(aiProcess_Triangulate | aiProcess_FlipUVs)
    );
    if (!ai_scene || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !ai_scene->mRootNode)
        return std::unexpected(std::format("Assimp: {}", importer.GetErrorString()));

    auto model = load_model(engine, ai_scene, model_dir);
    if (!model) return std::unexpected(model.error());
    scene.model = std::move(*model);

    return scene;
}

int main(int argc, char *argv[]) {
    auto result = run_app(
        argc, argv, "SDL3 21 - Model Loading", WINDOW_WIDTH, WINDOW_HEIGHT, BACKGROUND_COLOR,
        [](engine_t &engine) { return create_scene(engine); }
    );
    if (!result) {
        std::println(stderr, "{}", result.error());
        return 1;
    }
    return 0;
}
