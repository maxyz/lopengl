#include "common/mesh.hpp"
#include <cstddef>
#include <format>

void Mesh::setup_mesh() {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);
  // vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  // vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, normal)));
  // vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, tex_coords)));
  glBindVertexArray(0);
}

constexpr size_t first_texture_number = 1;

static std::string material_uniform_name(std::string_view type,
                                         size_t &diffuse_count,
                                         size_t &specular_count) {
  if (type == texture_type_diffuse)
    return std::format("material.{}{}", type, diffuse_count++);
  if (type == texture_type_specular)
    return std::format("material.{}{}", type, specular_count++);
  return std::format("material.{}", type);
}

void Mesh::draw(Shader &shader) {
  size_t diffuse_count = first_texture_number;
  size_t specular_count = first_texture_number;
  for (size_t i = 0; i < textures.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    shader.set_int(
        material_uniform_name(textures[i].type, diffuse_count, specular_count),
        i);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }
  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
