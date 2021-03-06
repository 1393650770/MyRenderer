#pragma once

#ifndef _MODEL_
#define _MODEL_

#define STB_IMAGE_IMPLEMENTATION
#include"../../ThirdParty/stb_image/stb_image.h"

#include <glad/glad.h>                 //所有头文件 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>        //assimp库头文件
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "../RHI/OpenGL/GL_Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;
namespace MXRender
{
    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

    class Model
    {
    public:

        vector<Texture> textures_loaded;	
        vector<Mesh>    meshes;
        string directory;
        bool gammaCorrection;

        Model(string const& path, bool gamma = false);

        void Draw(GL_Shader& shader);

    private:
    
        void loadModel(string const& path);

        void processNode(aiNode* node, const aiScene* scene);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
    };

}
#endif


