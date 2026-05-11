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

    Mesh(MeshVertices vertices, MeshIndices indices, MeshTextures textures);
    void draw(Shader &shader);

private:

    uint VAO, VBO, EBO;
    void setupMesh();

};