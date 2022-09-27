#pragma once
#ifndef _SHADER_
#define _SHADER_

#include <string>
#include<vector>
#include<memory>
#include"glm/glm.hpp"
#include "RenderEnum.h"

namespace MXRender
{
    class Shader
    {
    private:
    protected:
        // 程序ID
        unsigned int ID;
    public:
        Shader()=default;
        virtual ~Shader();

        virtual unsigned get_id() const = 0;

        virtual void bind() const =0; // uniform工具函数
        virtual void unbind() const = 0; // uniform工具函数
        virtual void setBool(const std::string& name, bool value)const = 0;
        virtual void setInt(const std::string& name, int value)const = 0;
        virtual void setFloat(const std::string& name, float value)const = 0;
        virtual void setVec2(const std::string& name, const glm::vec2& value) const = 0;
        virtual void setVec2(const std::string& name, float x, float y) const = 0;
        virtual void setVec3(const std::string& name, const glm::vec3& value) const = 0;
        virtual void setVec3(const std::string& name, float x, float y, float z) const = 0;
        virtual void setVec4(const std::string& name, const glm::vec4& value) const = 0;
        virtual void setVec4(const std::string& name, float x, float y, float z, float w) const = 0;
        virtual void setMat2(const std::string& name, const glm::mat2& mat) const = 0;
        virtual void setMat3(const std::string& name, const glm::mat3& mat) const = 0;
        virtual void setMat4(const std::string& name, const glm::mat4& mat) const = 0;
        virtual void SetUniform3f(const char* paraNameString, glm::vec3 param) = 0;
        virtual void SetUniform1f(const char* paraNameString, float param) = 0;
        virtual void SetUniform2f(const char* paraNameString, float param1, float param2) = 0;
        virtual void SetUniform1i(const char* paraNameString, int slot) = 0;

        //New api
        virtual void addUnifromName(const std::string& name, uint64_t uniformSize) = 0;


        static std::shared_ptr<Shader> CreateShader(const char* vertexPath = nullptr, const char* fragmentPath = nullptr, const char* geometryPath = nullptr, const char* computePath = nullptr);
    
    };


}
#endif //_SHADER_
