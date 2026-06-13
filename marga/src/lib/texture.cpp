#include "texture.h"

// Legacy constructor
Texture::Texture(const char* filename, GLenum format): Texture(filename) { }

Texture::Texture(const char* filename)
{
    glGenTextures(1, &ID);
    // Bind a texture into that integer
    glBindTexture(GL_TEXTURE_2D, ID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    // Load the image to use as texture
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename, &width, &height, &nrComponents, 0); 
    GLenum format;
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    if (data) {
        // Generate a texture and a mipmap based on the image
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "ERROR: Failed to load texture file: " << filename << std::endl;
    }
    stbi_image_free(data);
}

void Texture::flip_vertically() {
    stbi_set_flip_vertically_on_load(true);
}

void Texture::set_wrap(GLenum wrap_value)
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_value);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_value);

}

void Texture::set_filter(GLenum filter_value)
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_value);
}

CubeTexture::CubeTexture(std::vector<std::string> textures_faces)
{
    glGenTextures(1, &ID);
    // Bind a texture into that integer
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    // Load the image to use as texture
    int width, height, nrComponents;
    unsigned char *data;
    for(unsigned int i = 0; i < textures_faces.size(); i++)
    {
        data = stbi_load(textures_faces[i].c_str(), &width, &height, &nrComponents, 0);
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        if (data) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
        } else {
            std::cout << "ERROR: Failed to load cubemap texture file: " << textures_faces[i] << std::endl;
        }
        stbi_image_free(data);
    }   
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
}

void CubeTexture::flip_vertically() {
    stbi_set_flip_vertically_on_load(true);
}

void CubeTexture::set_wrap(GLenum wrap_value)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_value);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_value);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_value);   

}

void CubeTexture::set_filter(GLenum filter_value)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter_value);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter_value);
}


