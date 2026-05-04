#pragma once

#include <string>
#include <string_view>

#include <glm/glm.hpp>
#include <vector>

#include "common/shader.hpp"
#include "common/types.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
};

struct Texture {
  id_t id;
  std::string type;
};

constexpr std::string_view texture_type_diffuse = "texture_diffuse";
constexpr std::string_view texture_type_specular = "texture_specular";

class Mesh {
public:
  // mesh data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures)
      : vertices(vertices), indices(indices), textures(textures) {
    setup_mesh();
  };
  void draw(Shader &shader);

private:
  id_t m_vertex_array{};
  id_t m_vertex_buffer{};
  id_t m_element_buffer{};

  void setup_mesh();
};
