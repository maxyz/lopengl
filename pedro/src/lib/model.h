#ifndef MODEL_H
#define MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb/stb_image.h>

#include "mesh.h"

using ModelMeshes = std::vector<Mesh>;
using ModelTextures = std::vector<Texture>;

class Model {

public:

    Model(std::string path)
    {
        loadModel(path);
    }

    void draw(Shader &shader);

private:

    ModelTextures loadedTextures;
    ModelMeshes meshes;
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    MeshTextures loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    uint textureFromFile(std::string path, std::string directory);

};

struct VertexAttribInfo {
    uint location;
    uint size;
    uint stride;
    uintptr_t pointer;
};

using AttribInfo = std::vector<VertexAttribInfo>;
class VertexVector {
    
private:
    std::vector<float>* vertices;

public:
    AttribInfo attribInfo;
    uint triangleVertexAmount;

    VertexVector(std::vector<float> &vector, AttribInfo attribInfo, uint triangleVertexAmount) : attribInfo(attribInfo), triangleVertexAmount(triangleVertexAmount) {
        vertices = &vector;
    }

    size_t size() {
        return vertices->size();
    }

    float* data() {
        return vertices->data();
    }
};

#endif