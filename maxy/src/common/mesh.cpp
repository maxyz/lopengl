#include "common/mesh.hpp"
#include <cstddef>
#include <format>
#include <iostream>

void Mesh::setup_mesh() {
    glGenVertexArrays(1, &m_vertex_array);
    glGenBuffers(1, &m_vertex_buffer);
    glGenBuffers(1, &m_element_buffer);
    glBindVertexArray(m_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0],
        GL_STATIC_DRAW
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void *>(offsetof(Vertex, tex_coords))
    );
    glBindVertexArray(0);
}

constexpr size_t first_texture_number = 1;

struct texture_counters_t {
    size_t diffuse;
    size_t specular;
    size_t height;
    size_t ambient;
};

static std::string material_uniform_name(std::string_view type, texture_counters_t &counters) {
    if (type == texture_type_diffuse) return std::format("{}{}", type, counters.diffuse++);
    if (type == texture_type_specular) return std::format("{}{}", type, counters.specular++);
    if (type == texture_type_height) return std::format("{}{}", type, counters.height++);
    if (type == texture_type_ambient) return std::format("{}{}", type, counters.ambient++);
    return std::string(type);
}

void Mesh::draw(Shader &shader) {
    texture_counters_t counters{
        first_texture_number, first_texture_number, first_texture_number, first_texture_number
    };
    for (size_t i = 0; i < m_textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        shader.set_int(material_uniform_name(m_textures[i].type, counters), i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(m_vertex_array);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
