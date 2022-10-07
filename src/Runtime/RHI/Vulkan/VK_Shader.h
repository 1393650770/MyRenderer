#pragma once
#ifndef _VK_SHADER_
#define _VK_SHADER_

#include<vulkan/vulkan.h>
#include <string>
#include<vector>
#include<unordered_map>
#include<memory>
#include<tuple>
#include"glm/glm.hpp"
#include "../RenderEnum.h"
#include "../Shader.h"

namespace MXRender
{
    class VK_Device;

    class VK_Shader :public Shader
    {
    private:
        
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& getUniformTuple(const std::string& name);
    protected:
        // 程序ID
        unsigned int ID;
        
        std::weak_ptr<VK_Device> Device;

        mutable std::unordered_map<std::string,std::tuple<VkDeviceSize,VkBuffer, VkDeviceMemory>> Uniformmap;

    public:
        VkShaderModule shader_modules[ENUM_SHADER_STAGE::NumStages];
        
        VK_Shader(std::shared_ptr<VK_Device> InDevice, VkShaderStageFlagBits InStageFlag, const std::string& vertexPath = "", const std::string& fragmentPath = "", const std::string& geometryPath = "", const std::string& computePath = "");
        
        virtual ~VK_Shader();

        virtual unsigned get_id() const ;
        
        virtual void bind() const ; // uniform工具函数
        virtual void unbind() const ; // uniform工具函数
        virtual void setBool(const std::string& name, bool value) const;
        virtual void setInt(const std::string& name, int value)const ;
        virtual void setFloat(const std::string& name, float value)const ;
        virtual void setVec2(const std::string& name, const glm::vec2& value) const ;
        virtual void setVec2(const std::string& name, float x, float y) const ;
        virtual void setVec3(const std::string& name, const glm::vec3& value) const ;
        virtual void setVec3(const std::string& name, float x, float y, float z) const ;
        virtual void setVec4(const std::string& name, const glm::vec4& value) const ;
        virtual void setVec4(const std::string& name, float x, float y, float z, float w) const ;
        virtual void setMat2(const std::string& name, const glm::mat2& mat) const ;
        virtual void setMat3(const std::string& name, const glm::mat3& mat) const ;
        virtual void setMat4(const std::string& name, const glm::mat4& mat) const ;
        virtual void SetUniform3f(const char* paraNameString, glm::vec3 param) ;
        virtual void SetUniform1f(const char* paraNameString, float param) ;
        virtual void SetUniform2f(const char* paraNameString, float param1, float param2) ;
        virtual void SetUniform1i(const char* paraNameString, int slot) ;

        virtual void addUniformName(const std::string& name, uint64_t uniformSize) override;
    };


}
#endif //_VK_SHADER_
