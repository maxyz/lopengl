#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb/stb_image.h>
#include <iostream>
#include <glad/glad.h> 

enum mediaFormat {JPG, PNG};

class Texture2D
{
private:

    inline void createTexture() {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

public:
    unsigned int texture;

    Texture2D(const GLchar* mediaPath, const mediaFormat format, const uint wrap_s = GL_REPEAT, const uint wrap_t = GL_REPEAT, const uint min_filter = GL_LINEAR_MIPMAP_LINEAR, const uint mag_filter = GL_LINEAR);

    Texture2D(const uint width, const uint height, const uint min_filter = GL_LINEAR, const uint mag_filter = GL_LINEAR);

    void activate();
    void activate(unsigned int index);
};

#endif