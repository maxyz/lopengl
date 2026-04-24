#include "texture.h"

Texture2D::Texture2D(const GLchar* mediaPath, const mediaFormat format) {

    glGenTextures(1, &(this->texture));
    glBindTexture(GL_TEXTURE_2D, this->texture);

    // parametros bastante standard para el wrapping y filtering
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(mediaPath, &width, &height, &nrChannels, 0);    

    if (data)
    {

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

void Texture2D::activate() { this->activate(0); }

void Texture2D::activate(unsigned int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, this->texture);
}
