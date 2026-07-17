#include "buffers.h"

VAO::VAO(VertexVector &verticesVector, uint mode) {
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
    
    renderVertices = verticesVector.triangleVertexAmount;
    glBindVertexArray(0);
}

void Framebuffer::attatchColor() {
    colorAttachment = new Texture2D(width, height);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->texture, 0);
}

void Framebuffer::attatchRender() {
    glGenRenderbuffers(1, &renderAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, renderAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderAttachment);
}

void Framebuffer::checkStatus() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
}

void Framebuffer::completeGenerate(const uint _width, const uint _height){
    generate(_width, _height);
    bind();
    attatchColor();
    attatchRender();
    checkStatus();
    unbind();
}

void Framebuffer::resize(const uint _width, const uint _height) {
    bool dirty = false;
    if (width != _width) {
        width = _width;
        dirty = true;
    }
    if (height != _height) {
        height = _height;
        dirty = true;
    }

    if (dirty) {
        attatchColor();
        attatchRender();
    }
}
