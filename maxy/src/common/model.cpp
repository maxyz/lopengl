#include <filesystem>
#include <format>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "common/assets.hpp"
#include "common/mesh.hpp"
#include "common/model.hpp"

namespace fs = std::filesystem;

std::vector<Texture> Model::m_textures_loaded;

void Model::draw(Shader &shader) {
  for (auto &mesh : m_meshes) {
    mesh.draw(shader);
  }
}

std::expected<Model, std::string> Model::load(const std::string &path) {
  Model model;
  return model.load_model(path).transform(
      [&model] { return std::move(model); });
}

std::expected<void, std::string> Model::load_model(const std::string &path) {
  Assimp::Importer importer;
  m_directory = fs::path(path).parent_path();
  auto filepath = get_asset_path(path);
  if (!filepath) {
    return std::unexpected(filepath.error());
  }
  const aiScene *scene =
      importer.ReadFile(*filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    return std::unexpected(
        std::format("assimp: {}", importer.GetErrorString()));
  }
  return process_node(scene->mRootNode, scene);
}

std::expected<void, std::string> Model::process_node(aiNode *node,
                                                     const aiScene *scene) {
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    auto processed_mesh = process_mesh(mesh, scene);
    if (!processed_mesh) {
      return std::unexpected(
          std::format("mesh[{}]: {}", i, processed_mesh.error()));
    }
    m_meshes.push_back(std::move(*processed_mesh));
  }
  for (size_t i = 0; i < node->mNumChildren; i++) {
    auto result = process_node(node->mChildren[i], scene);
    if (!result) {
      return std::unexpected(std::format("node[{}]: {}", i, result.error()));
    }
  }
  return {};
}

std::expected<Mesh, std::string> Model::process_mesh(aiMesh *mesh,
                                                     const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  for (size_t i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;
    vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                mesh->mVertices[i].z);
    vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                              mesh->mNormals[i].z);
    if (mesh->mTextureCoords[0]) {
      vertex.tex_coords =
          glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }
    vertices.push_back(vertex);
  }

  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    for (size_t j = 0; j < face.mNumIndices; ++j) {
      indices.push_back(face.mIndices[j]);
    }
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    auto diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE,
                                               "texture_diffuse");
    if (!diffuse_maps) {
      return std::unexpected(
          std::format("diffuse_maps: {}", diffuse_maps.error()));
    }
    textures.insert(textures.end(), diffuse_maps->begin(), diffuse_maps->end());
    auto specular_maps = load_material_textures(
        material, aiTextureType_SPECULAR, "texture_specular");
    if (!specular_maps) {
      return std::unexpected(
          std::format("specular_maps: {}", specular_maps.error()));
    }
    textures.insert(textures.end(), specular_maps->begin(),
                    specular_maps->end());
  }
  return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::expected<std::vector<Texture>, std::string>
Model::load_material_textures(aiMaterial *material, aiTextureType type,
                              std::string_view name) {
  std::vector<Texture> textures;
  for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
    aiString str;
    material->GetTexture(type, i, &str);

    bool skip = false;
    for (const Texture &cached : m_textures_loaded) {
      if (cached.path == str.C_Str()) {
        textures.push_back(cached);
        skip = true;
        break;
      }
    }

    if (!skip) {
      auto id = load_texture(str.C_Str(), m_directory.string());
      if (!id) {
        return std::unexpected(
            std::format("image {}: {}", str.C_Str(), id.error()));
      }
      m_textures_loaded.push_back({.id = *id,
                                   .type = std::string(name),
                                   .path = str.C_Str()});
      textures.push_back(m_textures_loaded.back());
    }
  }
  return textures;
}
