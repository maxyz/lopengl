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

void Framebuffer::attatchColor(const uint width, const uint height) {
    colorAttachment = new Texture2D(width, height);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->texture, 0);
}

void Framebuffer::attatchRender(const uint width, const uint height) {
    glGenRenderbuffers(1, &renderAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, renderAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderAttachment);
}

void Framebuffer::checkStatus() {
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
}

void Framebuffer::completeGenerate(const uint width, const uint height){
    generate();
    bind();
    attatchColor(width, height);
    attatchRender(width, height);
    checkStatus();
    unbind();
}


