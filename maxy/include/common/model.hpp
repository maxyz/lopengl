#pragma once

#include <expected>
#include <string>
#include <vector>

#include <assimp/scene.h>

#include "common/mesh.hpp"
#include "common/shader.hpp"

class Model {
public:
  Model() = default;

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;
  Model(Model &&o) noexcept = default;
  Model &operator=(Model &&) = default;

  static std::expected<Model, std::string> load(const std::string &path);
  void draw(Shader &shader);

private:
  std::vector<Mesh> m_meshes;
  std::string m_directory;

  std::expected<void, std::string> load_model(const std::string &path);
  std::expected<void, std::string> process_node(aiNode *node,
                                                const aiScene *scene);
  std::expected<Mesh, std::string> process_mesh(aiMesh *mesh,
                                                const aiScene *scene);
  std::expected<std::vector<Texture>, std::string>
  load_material_textures(aiMaterial *material, aiTextureType type,
                         std::string name);
};
