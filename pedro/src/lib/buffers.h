#ifndef GLBUFFERS_H
#define GLBUFFERS_H

#include "model.h"
#include "texture.h"

class VAO {

private:
    uint vao, vbo;
    
public:

    uint renderVertices;
    VAO() : vao(0), vbo(0) {}
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
    uint width, height;

    void attatchColor();
    void attatchRender();

public:
    Texture2D* colorAttachment;

    Framebuffer() : colorAttachment(nullptr) {}
    ~Framebuffer() {
        if (colorAttachment != nullptr) delete colorAttachment;
    }

    void generate(const uint _width, const uint _height) {
        glGenFramebuffers(1, &buffer);
        width = _width;
        height = _height;
    }

    void checkStatus();

    void completeGenerate(const uint _width, const uint _height);

    void resize(const uint _width, const uint _height);

    inline void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer);
    }
    inline void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    inline void deleteBuffers() {
        glDeleteFramebuffers(1, &buffer);
        glDeleteRenderbuffers(1, &renderAttachment);
        if (colorAttachment != nullptr) colorAttachment->deleteTexture();
    }
};

#endif