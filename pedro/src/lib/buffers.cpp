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

Framebuffer::Framebuffer(){
    glGenFramebuffers(1, &buffer);
    colorAttatchment = nullptr;
}

void Framebuffer::attatchColor(const uint width, const uint height) {
    colorAttatchment = new Texture2D(width, height);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttatchment->texture, 0);
}

void Framebuffer::attatchRender(const uint width, const uint height) {
    glGenRenderbuffers(1, &renderAttatchment);
    glBindRenderbuffer(GL_RENDERBUFFER, renderAttatchment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderAttatchment);
}

void Framebuffer::checkStatus() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
}

