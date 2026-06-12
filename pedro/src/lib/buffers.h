#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include "model.h"

class VAO {

public:
    unsigned int vao, vbo;

    VAO(VertexVector &verticesVector, unsigned int mode = GL_STATIC_DRAW) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verticesVector.size, &verticesVector.vertices, GL_STATIC_DRAW);

        for (auto info : verticesVector.attribInfo)
        {
            auto& [location, size, stride, pointer] = info;
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, (void*)pointer);
        }

        glBindVertexArray(0);
    }

    inline void bind() {
        glBindVertexArray(vao);
    }
    inline void unbind() {
        glBindVertexArray(0);
    }
    inline void deleteBuffers() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }
};

#endif