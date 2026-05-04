#pragma once

#include <expected>
#include <string>
#include <vector>

#include "common/mesh.hpp"
#include "common/shader.hpp"

class Model {
public:
  Model() = default;
  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;
  Model(Model &&) noexcept = default;
  Model &operator=(Model &&) = default;

  static std::expected<Model, std::string> load(const std::string &path);
  void draw(Shader &shader);

private:
  std::vector<Mesh> m_meshes;
  static std::vector<Texture> m_textures_loaded;
};
