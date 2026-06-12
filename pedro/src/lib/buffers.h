#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include "model.h"

class VAO {

private:
    

public:
    unsigned int vao, vbo;

    VAO(VertexVector &verticesVector, unsigned int mode = GL_STATIC_DRAW) {
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