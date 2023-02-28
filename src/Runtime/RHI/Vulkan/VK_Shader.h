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
#include "vulkan/vulkan_core.h"
#include <array>

namespace MXRender
{
    class VK_Device;

    class VK_Shader :public Shader
    {
    public:
		struct ReflectionOverrides {
			const char* name;
			VkDescriptorType overridenType;
		};
		struct DescriptorSetLayoutData {
			uint32_t set_number;
			VkDescriptorSetLayoutCreateInfo create_info;
			std::vector<VkDescriptorSetLayoutBinding> bindings;
		};
		struct ReflectedBinding {
			uint32_t set;
			uint32_t binding;
			VkDescriptorType type;
		};
    private:
        
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& getUniformTuple(const std::string& name);
    protected:
        // ����ID
        unsigned int ID;
        
        std::weak_ptr<VK_Device> Device;

        mutable std::unordered_map<std::string,std::tuple<VkDeviceSize,VkBuffer, VkDeviceMemory>> Uniformmap;

		std::unordered_map<std::string, ReflectedBinding> Bindings;
		std::array<VkDescriptorSetLayout, 4> setLayouts;
		std::array<uint32_t, 4> setHashes;
        std::array<VkDescriptorSet,4> sets;
        VkPipelineLayout BuiltLayout = VK_NULL_HANDLE;
    public:
        VkShaderModule shader_modules[ENUM_SHADER_STAGE::NumStages]{VK_NULL_HANDLE};
        std::vector<char> shader_codes[ENUM_SHADER_STAGE::NumStages];

        VK_Shader(std::shared_ptr<VK_Device> InDevice, VkShaderStageFlagBits InStageFlag, const std::string& vertexPath = "", const std::string& fragmentPath = "", const std::string& geometryPath = "", const std::string& computePath = "");
        
        VkPipelineLayout get_built_layout();

        void fill_stages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages);
        void reflect_layout( ReflectionOverrides* overrides, int overrideCount);
        void build_sets(VkDevice device , VkDescriptorPool descript_pool);
        void destroy();
        virtual ~VK_Shader();

        virtual unsigned get_id() const ;
        
        virtual void bind() const ; // uniform���ߺ���
        virtual void unbind() const ; // uniform���ߺ���
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
