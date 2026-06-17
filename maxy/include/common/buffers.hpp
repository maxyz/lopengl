#pragma once

#include <span>

#include <glad/gl.h>

#include <glm/gtc/type_ptr.hpp>

#include "geometry.hpp"
#include "types.hpp"

namespace buffers {
void load_vertices(
    std::span<const vertex_t> vertices, id_t vertex_array_object, id_t vertex_buffer_object
);

void uniform_block_alloc(id_t uniform_buffer, id_t index, size_t size);

template <typename T> void uniform_block_memcpy(id_t uniform_buffer, size_t offset, const T &data) {
    static_assert(
        std::is_trivially_copyable_v<T>, "Data must be trivially copyable to be sent to the GPU"
    );
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T), &data);
}

} // namespace buffers
