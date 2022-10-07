#pragma once

#ifndef _GL_SHADER_
#define _GL_SHADER_

#include"../Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include"../RenderEnum.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace MXRender
{
    class GL_Shader:public Shader
    {
    public:
        // 构造器读取并构建着色器 
        GL_Shader(const GLchar* vertexPath=nullptr, const GLchar* fragmentPath=nullptr, const GLchar* geometryPath=nullptr,const GLchar* computePath=nullptr);
        ~GL_Shader();
        unsigned get_id() const override;
        // 使用/激活程序
        void bind() const override; // uniform工具函数
        void unbind() const override; // uniform工具函数
        void setBool(const std::string& name, bool value)const override;
        void setInt(const std::string& name, int value)const override;
        void setFloat(const std::string& name, float value)const override;
        void setVec2(const std::string& name, const glm::vec2& value) const override;
        void setVec2(const std::string& name, float x, float y) const override;
        void setVec3(const std::string& name, const glm::vec3& value) const override;
        void setVec3(const std::string& name, float x, float y, float z) const override;
        void setVec4(const std::string& name, const glm::vec4& value) const override;
        void setVec4(const std::string& name, float x, float y, float z, float w) const override;
        void setMat2(const std::string& name, const glm::mat2& mat) const override;
        void setMat3(const std::string& name, const glm::mat3& mat) const override;
        void setMat4(const std::string& name, const glm::mat4& mat) const override;
        void SetUniform3f(const char* paraNameString, glm::vec3 param) override;
        void SetUniform1f(const char* paraNameString, float param) override;
        void SetUniform2f(const char* paraNameString, float param1, float param2) override;
        void SetUniform1i(const char* paraNameString, int slot) override;

        virtual void addUniformName(const std::string& name, uint64_t uniformSize) ;

    };
}
#endif
