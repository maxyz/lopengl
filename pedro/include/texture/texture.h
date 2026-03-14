#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "../src/04_textures/stb_image/stb_image.h"
#include <iostream>
#include <glad/glad.h> 

enum mediaFormat {JPG, PNG};

class Texture2D
{
public:
    unsigned int texture;

    Texture2D(const GLchar* mediaPath, const mediaFormat format, const bool flip);

    void Activate();
    void Activate(unsigned int index);
};

Texture2D::Texture2D(const GLchar* mediaPath, const mediaFormat format, const bool flip) {

    glGenTextures(1, &(this->texture));
    glBindTexture(GL_TEXTURE_2D, this->texture);

    // parametros bastante standard para el wrapping y filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(mediaPath, &width, &height, &nrChannels, 0);    

    if (data)
    {
        stbi_set_flip_vertically_on_load(flip);

        switch (format)
        {
        case PNG:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;
        
        case JPG:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            break;
        }
    } else {
        std::cout << "ERROR::TEXTURE::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
}

void Texture2D::Activate() { this->Activate(0); }

void Texture2D::Activate(unsigned int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}

#endif