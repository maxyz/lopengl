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