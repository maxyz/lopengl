#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb/stb_image.h>
#include <iostream>
#include <glad/glad.h> 

enum mediaFormat {JPG, PNG};

class Texture2D
{
private:

    void createTexture() {
        glGenTextures(1, &this->texture);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

public:
    uint texture;

    Texture2D() : texture(0) {}
    Texture2D(const GLchar* mediaPath, const mediaFormat format, const uint wrap_s = GL_REPEAT, const uint wrap_t = GL_REPEAT, const uint min_filter = GL_LINEAR_MIPMAP_LINEAR, const uint mag_filter = GL_LINEAR);
    ~Texture2D() {
        glDeleteTextures(1, &texture);
    }

    Texture2D(const uint width, const uint height, const uint min_filter = GL_LINEAR, const uint mag_filter = GL_LINEAR);

    void activate();
    void activate(unsigned int index);
};

#endif