#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

#include "common/model.hpp"

void Model::draw(Shader &shader) {
  for (auto mesh : m_meshes) {
    mesh.draw(shader);
  }
}

void Model::load_model(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "Error assimp" << importer.GetErrorString() << std::endl;
    return;
  }
  m_directory = path.substr(0, path.find_last_of('/'));

  process_node(scene->mRootNode, scene);
}
