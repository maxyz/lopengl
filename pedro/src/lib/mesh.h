#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <vector>

#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    uint id;
    std::string type;
    std::string path;
};

using MeshVertices = std::vector<Vertex>;
using MeshIndices = std::vector<uint>;
using MeshTextures = std::vector<Texture>;

class Mesh {

public:

    MeshVertices vertices;
    MeshIndices indices;
    MeshTextures textures;

    Mesh() : nextArrayIndex(0) {}
    Mesh(MeshVertices vertices, MeshIndices indices, MeshTextures textures);
    void draw(Shader &shader, uint instances);
    void addInstancing(std::vector<glm::mat4> vertices);

private:

    uint nextArrayIndex;
    uint VAO, VBO, EBO;
    void setupMesh();

};

#endif