#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <format>

#include "common/model.hpp"

void Model::draw(Shader &shader) {
  for (auto &mesh : m_meshes) {
    mesh.draw(shader);
  }
}

std::expected<Model, std::string> Model::load(const std::string &path) {
  Model model;
  return model.load_model(path).transform([&model] { return std::move(model); });
}

std::expected<void, std::string> Model::load_model(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    return std::unexpected(
        std::format("assimp: {}", importer.GetErrorString()));
  }
  m_directory = path.substr(0, path.find_last_of('/'));
  return process_node(scene->mRootNode, scene);
}
