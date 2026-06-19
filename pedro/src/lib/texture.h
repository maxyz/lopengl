#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb/stb_image.h>
#include <iostream>
#include <vector>
#include <glad/glad.h> 

enum mediaFormat {JPG, PNG};

// Abstract class
class AbstractTexture
{
protected:
    void createTexture(uint idx) {
        glGenTextures(1, &texture);
        index = idx;
    }

public:
    uint texture;
    uint index;

    AbstractTexture() = default;
    ~AbstractTexture() = default;

    virtual void activate() = 0;
    virtual void activate(unsigned int) = 0;

    void deleteTexture() {
        glDeleteTextures(1, &texture);
    }
};

class Texture2D : public AbstractTexture
{
public:

    Texture2D() = default;
    Texture2D(const GLchar* mediaPath, const mediaFormat format, const uint idx = 0, const uint wrap_s = GL_REPEAT, const uint wrap_t = GL_REPEAT, const uint min_filter = GL_LINEAR_MIPMAP_LINEAR, const uint mag_filter = GL_LINEAR);
    Texture2D(const uint width, const uint height, const uint idx = 0, const uint min_filter = GL_LINEAR, const uint mag_filter = GL_LINEAR);

    void activate() override;
    void activate(unsigned int) override;
};

class Cubemap : public AbstractTexture
{
public:

    Cubemap() = default;
    Cubemap(const std::vector<std::string>&, const uint idx = 0);

    void activate() override;
    void activate(unsigned int) override;
};

#endif