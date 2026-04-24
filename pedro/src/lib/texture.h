#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb/stb_image.h>
#include <iostream>
#include <glad/glad.h> 

enum mediaFormat {JPG, PNG};

class Texture2D
{
public:
    unsigned int texture;

    Texture2D(const GLchar* mediaPath, const mediaFormat format);

    void activate();
    void activate(unsigned int index);
};

#endif