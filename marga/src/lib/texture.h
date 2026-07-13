#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "stb_image.h"

class Texture
{
public:
    // the texture ID
    unsigned int ID;

    // constructor reads and builds the shader
    Texture(const char* filename);
    Texture(const char* filename, GLenum format);
    void init(const char* filename);

    // flip orientation
    static void flip_vertically();

    // Change the wrap values
    void set_wrap(GLenum wrap_value);

    // Change the filtering values
    void set_filter(GLenum filter_value);

    // Used by the model loader
    Texture(const char *filepath, const std::string &directory);
    std::string type;
    std::string path;
};

class CubeTexture
{
public:
    // the texture ID
    unsigned int ID;

    // constructor reads and builds the shader
    CubeTexture(std::vector<std::string> textures_faces);

    // flip orientation
    static void flip_vertically();

    // Change the wrap values
    void set_wrap(GLenum wrap_value);

    // Change the filtering values
    void set_filter(GLenum filter_value);
};
#endif
