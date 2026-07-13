#ifndef __MODEL_H
#define __MODEL_H

#include <vector>
#include <string>

#include "shader.h"
#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model 
{
    public:
        Model(std::string path);
        void Draw(Shader &shader);	
        std::vector<Mesh> meshes;
        std::vector<Texture> textures_loaded;
    private:
        // model data
        std::string directory;

        void loadModel(std::string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, 
                                             std::string typeName);
};

#endif
