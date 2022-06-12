#pragma once
#ifndef _MESH_
#define _MESH_
#include <glad/glad.h> // 所有头文件
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../RHI/GL_Shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

namespace MXRender
{
    #define MAX_BONE_INFLUENCE 4

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
    };

    struct Texture {
        unsigned int id;
        string type;
        string path;
    };

    class Mesh {
    public:
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<Texture>      textures;
        unsigned int VAO;

        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
        void DrawArray(GL_Shader* shader, int diffuse, int specular, int emission);
        Mesh(float vertices[]);

        void Draw(GL_Shader& shader);

    private:

        unsigned int VBO, EBO;

        void setupMesh();

    };
}
#endif

