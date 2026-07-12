#ifndef __MESH_H
#define __MESH_H

#include <string>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "stb_image.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Texture {
public:
    Texture(const char *filepath, const std::string &directory);
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
    public:
        // mesh data
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        void Draw(Shader &shader);

        unsigned int VAO;
    private:
        //  render data
        unsigned int VBO, EBO;
        void setupMesh();
};

#endif
