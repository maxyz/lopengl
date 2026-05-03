#pragma once

#include <string>
#include <vector>

#include <assimp/scene.h>

#include "common/mesh.hpp"
#include "common/shader.hpp"

class Model {
public:
  Model(const std::string &path) { load_model(path); }
  void draw(Shader &shader);

private:
  // model data
  std::vector<Mesh> m_meshes;
  std::string m_directory;

  void load_model(const std::string &path);
  void process_node(aiNode *node, const aiScene *scene);
  Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture> load_material_textures(aiMaterial *material,
                                              aiTextureType type,
                                              std::string name);
};
