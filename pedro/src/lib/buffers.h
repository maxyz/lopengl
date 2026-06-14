#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include "model.h"
#include "texture.h"

class VAO {

private:
    uint vao, vbo;
    
public:

    VAO(VertexVector &verticesVector, uint mode = GL_STATIC_DRAW);

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

class Framebuffer {

private:
    uint buffer;
    uint renderAttachment;
    

public:
    Texture2D* colorAttachment;

    Framebuffer();
    void attatchColor(const uint width, const uint height);
    void attatchRender(const uint width, const uint height);

    void checkStatus();

    inline void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    }
    inline void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

#endif