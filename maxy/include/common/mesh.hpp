#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

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
  std::string path;
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
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(Mesh &&o) noexcept
      : vertices(std::move(o.vertices)), indices(std::move(o.indices)),
        textures(std::move(o.textures)),
        m_vertex_array(std::exchange(o.m_vertex_array, 0)),
        m_vertex_buffer(std::exchange(o.m_vertex_buffer, 0)),
        m_element_buffer(std::exchange(o.m_element_buffer, 0)) {}
  Mesh &operator=(Mesh &&o) noexcept {
    if (this != &o) {
      glDeleteVertexArrays(1, &m_vertex_array);
      glDeleteBuffers(1, &m_vertex_buffer);
      glDeleteBuffers(1, &m_element_buffer);
      vertices = std::move(o.vertices);
      indices = std::move(o.indices);
      textures = std::move(o.textures);
      m_vertex_array = std::exchange(o.m_vertex_array, 0);
      m_vertex_buffer = std::exchange(o.m_vertex_buffer, 0);
      m_element_buffer = std::exchange(o.m_element_buffer, 0);
    }
    return *this;
  }

  ~Mesh() {
    glDeleteVertexArrays(1, &m_vertex_array);
    glDeleteBuffers(1, &m_vertex_buffer);
    glDeleteBuffers(1, &m_element_buffer);
  };
  void draw(Shader &shader);

private:
  id_t m_vertex_array{};
  id_t m_vertex_buffer{};
  id_t m_element_buffer{};

  void setup_mesh();
};
