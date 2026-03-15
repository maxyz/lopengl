#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "stb_image.h"

class Texture
{
public:
    // the texture ID
    unsigned int ID;

    // constructor reads and builds the shader
    Texture(const char* filename, GLenum format);

    // flip orientation
    static void flip_vertically();
};

#endif
