
#include <iostream>
#include <fstream>
#include"VK_Shader.h"
#include"Vk_Device.h"

namespace MXRender
{
	std::vector<char> VK_Shader::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule VK_Shader::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;

		if(Device.expired()==false)
		{ 
			if (vkCreateShaderModule(Device.lock()->Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create shader module!");
			}
		}
		return shaderModule;
	}


	VK_Shader::VK_Shader(std::shared_ptr<VK_Device> InDevice, VkShaderStageFlagBits InStageFlag, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath)
	{
		Device=InDevice;

	}

	VK_Shader::~VK_Shader()
	{
	}

	void VK_Shader::bind() const
	{
	}

	void VK_Shader::unbind() const
	{
	}

	void VK_Shader::setBool(const std::string& name, bool value) const
	{
	}

	void VK_Shader::setInt(const std::string& name, int value) const
	{
	}

	void VK_Shader::setFloat(const std::string& name, float value) const
	{
	}

	void VK_Shader::setVec2(const std::string& name, const glm::vec2& value) const
	{
	}

	void VK_Shader::setVec2(const std::string& name, float x, float y) const
	{
	}

	void VK_Shader::setVec3(const std::string& name, const glm::vec3& value) const
	{
	}

	void VK_Shader::setVec3(const std::string& name, float x, float y, float z) const
	{
	}

	void VK_Shader::setVec4(const std::string& name, const glm::vec4& value) const
	{
	}

	void VK_Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
	{
	}

	void VK_Shader::setMat2(const std::string& name, const glm::mat2& mat) const
	{
	}

	void VK_Shader::setMat3(const std::string& name, const glm::mat3& mat) const
	{
	}

	void VK_Shader::setMat4(const std::string& name, const glm::mat4& mat) const
	{
	}

	void VK_Shader::SetUniform3f(const char* paraNameString, glm::vec3 param)
	{
	}

	void VK_Shader::SetUniform1f(const char* paraNameString, float param)
	{
	}

	void VK_Shader::SetUniform2f(const char* paraNameString, float param1, float param2)
	{
	}

	void VK_Shader::SetUniform1i(const char* paraNameString, int slot)
	{
	}



}


