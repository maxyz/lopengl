#include "texture.h"

Texture2D::Texture2D(const GLchar* mediaPath, const mediaFormat format, const uint wrap_s, const uint wrap_t, const uint min_filter, const uint mag_filter) {
    createTexture();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(mediaPath, &width, &height, &nrChannels, 0);    

    if (data)
    {
        switch (format)
        {
        case PNG:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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

Texture2D::Texture2D(const uint width, const uint height, const uint min_filter, const uint mag_filter) {
    createTexture();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void Texture2D::activate() { this->activate(0); }

void Texture2D::activate(unsigned int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}
