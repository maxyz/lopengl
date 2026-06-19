#include "texture.h"

Texture2D::Texture2D(const GLchar* mediaPath, const mediaFormat format, const uint idx, const uint wrap_s, const uint wrap_t, const uint min_filter, const uint mag_filter) {
    createTexture(idx);

    glBindTexture(GL_TEXTURE_2D, texture);

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
        
        std::cout << "Tex2D load successful" << std::endl;
    } else {
        std::cout << "ERROR::TEXTURE::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);    
}

Texture2D::Texture2D(const uint width, const uint height, const uint idx, const uint min_filter, const uint mag_filter) {
    createTexture(idx);


    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void Texture2D::activate() 
{ 
    activate(index);
}

void Texture2D::activate(unsigned int idx)
{
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, texture);
}

Cubemap::Cubemap(const std::vector<std::string>& faces, const uint idx)
{
    createTexture(idx);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
            std::cout << "Cubemap tex load successful" << std::endl;
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Cubemap::activate()
{ 
    activate(index);
}
void Cubemap::activate(unsigned int idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, texture);
}