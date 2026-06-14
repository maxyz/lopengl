#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include "model.h"

class VAO {

private:
    unsigned int vao, vbo;
    
public:

    VAO(VertexVector &verticesVector, unsigned int mode = GL_STATIC_DRAW);

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