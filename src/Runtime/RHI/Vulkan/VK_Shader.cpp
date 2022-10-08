

#include"VK_Shader.h"
#include"Vk_Device.h"
#include <iostream>
#include <fstream>
#include"VK_Utils.h"


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


		if (vkCreateShaderModule(Device.lock()->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}
		
		return shaderModule;
	}

	std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& VK_Shader::getUniformTuple(const std::string& name)
	{
		return Uniformmap[name];
	}


	VK_Shader::VK_Shader(std::shared_ptr<VK_Device> InDevice, VkShaderStageFlagBits InStageFlag, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath)
	{
		Device=InDevice;
		if (Device.expired() == false)
		{
			if(vertexPath.size()>0&&vertexPath!="")
			{
				auto vertShaderCode = readFile(vertexPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Vertex]= createShaderModule(vertShaderCode);
			}
			if (fragmentPath.size() > 0 && fragmentPath != "")
			{
				
				auto pixelShaderCode = readFile(fragmentPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Pixel] = createShaderModule(pixelShaderCode);
			}
			if (geometryPath.size() > 0 && geometryPath != "")
			{

				auto geometryShaderCode = readFile(geometryPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Geometry] = createShaderModule(geometryShaderCode);
			}
			if (computePath.size() > 0 && computePath != "")
			{ 
				auto computeShaderCode = readFile(computePath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Compute] = createShaderModule(computeShaderCode);
			}
		}
	}

	VK_Shader::~VK_Shader()
	{
		if (Device.expired() == false)
		{
			for (size_t i = 0; i < ENUM_SHADER_STAGE::NumStages; i++)
			{
				if (shader_modules[i] != VK_NULL_HANDLE)
				{
					vkDestroyShaderModule(Device.lock()->device, shader_modules[i], nullptr);
				}
			}
		}
		
	}

	unsigned VK_Shader::get_id() const
	{
		return 0;
	}

	void VK_Shader::bind() const
	{
	}

	void VK_Shader::unbind() const
	{
	}

	void VK_Shader::setBool(const std::string& name, bool value) const
	{
		if (Device.expired()||Uniformmap.contains(name)==false)
		{
			return;
		}
		VkDeviceSize buffer_size= sizeof(bool);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return ;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]),0, buffer_size,0,&data);
		memcpy(data,&value,sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	
	}

	void VK_Shader::setInt(const std::string& name, int value) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(int);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &value, sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setFloat(const std::string& name, float value) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(float);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &value, sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setVec2(const std::string& name, const glm::vec2& value) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::vec2);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &value, sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setVec2(const std::string& name, float x, float y) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		glm::vec2 cur_vec2;
		cur_vec2.x=x;
		cur_vec2.y=y;
		setVec2(name,cur_vec2);
	}

	void VK_Shader::setVec3(const std::string& name, const glm::vec3& value) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::vec3);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &value, sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setVec3(const std::string& name, float x, float y, float z) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		glm::vec3 cur_vec3;
		cur_vec3.x = x;
		cur_vec3.y = y;
		cur_vec3.z = z;
		setVec3(name, cur_vec3);
	}

	void VK_Shader::setVec4(const std::string& name, const glm::vec4& value) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::vec4);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &value, sizeof(value));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		glm::vec4 cur_vec4;
		cur_vec4.x = x;
		cur_vec4.y = y;
		cur_vec4.z = z;
		cur_vec4.w = w; 
		setVec3(name, cur_vec4);
	}

	void VK_Shader::setMat2(const std::string& name, const glm::mat2& mat) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::mat2);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &mat, sizeof(mat));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setMat3(const std::string& name, const glm::mat3& mat) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::mat3);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &mat, sizeof(mat));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::setMat4(const std::string& name, const glm::mat4& mat) const
	{
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::mat4);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &mat, sizeof(mat));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::SetUniform3f(const char* paraNameString, glm::vec3 param)
	{
		std::string name(paraNameString);
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(glm::vec3);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &param, sizeof(param));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::SetUniform1f(const char* paraNameString, float param)
	{
		std::string name(paraNameString);
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		VkDeviceSize buffer_size = sizeof(float);
		if (buffer_size != std::get<0>(Uniformmap[name]))
		{
			return;
		}
		void* data;
		vkMapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]), 0, buffer_size, 0, &data);
		memcpy(data, &param, sizeof(param));
		vkUnmapMemory(Device.lock()->device, std::get<2>(Uniformmap[name]));
	}

	void VK_Shader::SetUniform2f(const char* paraNameString, float param1, float param2)
	{
		std::string name(paraNameString);
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		glm::vec2 cur_vec2;
		cur_vec2.x = param1;
		cur_vec2.y = param2;
		setVec2(name, cur_vec2);
	}

	void VK_Shader::SetUniform1i(const char* paraNameString, int slot)
	{
		std::string name(paraNameString);
		if (Device.expired() || Uniformmap.contains(name) == false)
		{
			return;
		}
		setInt(name,slot);
	}

	void VK_Shader::addUniformName(const std::string& name, uint64_t uniformSize)
	{
		Uniformmap[name] = std::make_tuple(uniformSize, VkBuffer(), VkDeviceMemory());
		VK_Utils::Create_VKBuffer(Device,uniformSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,std::get<1>(Uniformmap[name]), std::get<2>(Uniformmap[name]));
	}





}


