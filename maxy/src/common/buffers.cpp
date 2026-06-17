#include "common/buffers.hpp"

namespace buffers {
void uniform_block_alloc(id_t uniform_buffer, id_t index, size_t size) {
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<ssize_t>(size), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, index, uniform_buffer);
}

void load_vertices(
    std::span<const vertex_t> vertices, id_t vertex_array_object, id_t vertex_buffer_object
) {

    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, position))
    );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, normal))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
        reinterpret_cast<void *>(offsetof(vertex_t, tex_coord))
    );
    glEnableVertexAttribArray(2);
}
} // namespace buffers
