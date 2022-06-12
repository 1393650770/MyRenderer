#pragma once
#ifndef _GL_SHADER_
#define _GL_SHADER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"GL_ENUM.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
namespace MXRender
{
    class GL_Shader
    {
    public:
        // 程序ID
        unsigned int ID;
        // 构造器读取并构建着色器 
        GL_Shader(const GLchar* vertexPath=nullptr, const GLchar* fragmentPath=nullptr, const GLchar* geometryPath=nullptr,const GLchar* computePath=nullptr);
        ~GL_Shader();
        // 使用/激活程序
        void use(); // uniform工具函数
        void setBool(const std::string& name, bool value)const;
        void setInt(const std::string& name, int value)const;
        void setFloat(const std::string& name, float value)const;
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec2(const std::string& name, float x, float y) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3(const std::string& name, float x, float y, float z) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec4(const std::string& name, float x, float y, float z, float w) const;
        void setMat2(const std::string& name, const glm::mat2& mat) const;
        void setMat3(const std::string& name, const glm::mat3& mat) const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void SetUniform3f(const char* paraNameString, glm::vec3 param);
        void SetUniform1f(const char* paraNameString, float param);
        void SetUniform2f(const char* paraNameString, float param1, float param2);
        void SetUniform1i(const char* paraNameString, int slot);
    };
}
#endif
