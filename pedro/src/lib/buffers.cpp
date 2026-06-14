#include "buffers.h"

VAO::VAO(VertexVector &verticesVector, unsigned int mode) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesVector.size() * sizeof(float), verticesVector.data(), GL_STATIC_DRAW);

    for (auto info : verticesVector.attribInfo)
    {
        auto& [location, size, stride, pointer] = info;

        glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride * sizeof(float) , (void*) (pointer * sizeof(float)));
        glEnableVertexAttribArray(location);
    }

    glBindVertexArray(0);
}