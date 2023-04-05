

#include"VK_Shader.h"
#include"Vk_Device.h"
#include <iostream>
#include <fstream>
#include"VK_Utils.h"
#include "../../../ThirdParty/spv_reflect/spirv_reflect.h"
#include <algorithm>

namespace MXRender
{
	std::vector<uint32_t> VK_Shader::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VkShaderModule VK_Shader::createShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = (code.data());
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


	VK_Shader::VK_Shader(std::shared_ptr<VK_Device> InDevice, const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath, const std::string& computePath)
	{
		Device=InDevice;
		if (Device.expired() == false)
		{
			if(vertexPath.size()>0&&vertexPath!="")
			{
				shader_codes[ENUM_SHADER_STAGE::Shader_Vertex] = readFile(vertexPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Vertex]= createShaderModule(shader_codes[ENUM_SHADER_STAGE::Shader_Vertex]);
			}
			if (fragmentPath.size() > 0 && fragmentPath != "")
			{
				
				shader_codes[ENUM_SHADER_STAGE::Shader_Pixel] = readFile(fragmentPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Pixel] = createShaderModule(shader_codes[ENUM_SHADER_STAGE::Shader_Pixel]);
			}
			if (geometryPath.size() > 0 && geometryPath != "")
			{

				shader_codes[ENUM_SHADER_STAGE::Shader_Geometry] = readFile(geometryPath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Geometry] = createShaderModule(shader_codes[ENUM_SHADER_STAGE::Shader_Geometry]);
			}
			if (computePath.size() > 0 && computePath != "")
			{ 
				shader_codes[ENUM_SHADER_STAGE::Shader_Compute] = readFile(computePath);
				shader_modules[ENUM_SHADER_STAGE::Shader_Compute] = createShaderModule(shader_codes[ENUM_SHADER_STAGE::Shader_Compute]);
			}
		}
	}

	VkPipelineLayout VK_Shader::get_built_layout()
	{
		return BuiltLayout;
	}

	void VK_Shader::fill_stages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages)
	{
		for (int index = 0; index < ENUM_SHADER_STAGE::NumStages; index++)
		{
			if (shader_modules[index] != VK_NULL_HANDLE)
			{
				pipelineStages.push_back(VK_Utils::Pipeline_Shader_Stage_Create_Info(VK_Utils::Translate_API_ShaderTypeEnum_To_Vulkan((ENUM_SHADER_STAGE)index), shader_modules[index]));
			}
		}
	}

	void VK_Shader::reflect_layout(ReflectionOverrides* overrides, int overrideCount)
	{
		if (Device.expired())
		{
			return;
		}
		std::vector<DescriptorSetLayoutData> SetLayouts;
		std::vector<VkPushConstantRange> ConstantRanges;
		VkDevice device=Device.lock()->device;
		for (int index = 0 ;index< ENUM_SHADER_STAGE::NumStages;index++)
		{
			if (shader_modules[index]!=VK_NULL_HANDLE)
			{
				SpvReflectShaderModule spvmodule;
				SpvReflectResult result = spvReflectCreateShaderModule(shader_codes[index].size() * sizeof(uint32_t), shader_codes[index].data(), &spvmodule);

				uint32_t count = 0;
				result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				std::vector<SpvReflectDescriptorSet*> sets(count);
				result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				for (size_t i_set = 0; i_set < sets.size(); ++i_set) 
				{

					const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

					DescriptorSetLayoutData layout = {};

					layout.bindings.resize(refl_set.binding_count);
					for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
						const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
						VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
						layout_binding.binding = refl_binding.binding;
						layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

						for (int ov = 0; ov < overrideCount; ov++)
						{
							if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
								layout_binding.descriptorType = overrides[ov].overridenType;
							}
						}

						layout_binding.descriptorCount = 1;
						for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
							layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
						}
						layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);

						ReflectedBinding reflected;
						reflected.binding = layout_binding.binding;
						reflected.set = refl_set.set;
						reflected.type = layout_binding.descriptorType;

						Bindings[refl_binding.name] = reflected;
					}
					layout.set_number = refl_set.set;
					layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
					layout.create_info.bindingCount = refl_set.binding_count;
					layout.create_info.pBindings = layout.bindings.data();

					SetLayouts.push_back(layout);
				}

				result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				std::vector<SpvReflectBlockVariable*> pconstants(count);
				result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
				assert(result == SPV_REFLECT_RESULT_SUCCESS);

				if (count > 0) {
					VkPushConstantRange pcs{};
					pcs.offset = pconstants[0]->offset;
					pcs.size = pconstants[0]->size;
					pcs.stageFlags = VK_Utils::Translate_API_ShaderTypeEnum_To_Vulkan((ENUM_SHADER_STAGE)index);

					ConstantRanges.push_back(pcs);
				}
			}
		
		}


		std::array<DescriptorSetLayoutData, 4> merged_layouts;

		for (int i = 0; i < 4; i++) {

			DescriptorSetLayoutData& ly = merged_layouts[i];

			ly.set_number = i;

			ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

			std::unordered_map<int, VkDescriptorSetLayoutBinding> binds;
			for (auto& s : SetLayouts) {
				if (s.set_number == i) {
					for (auto& b : s.bindings)
					{
						auto it = binds.find(b.binding);
						if (it == binds.end())
						{
							binds[b.binding] = b;
							//ly.bindings.push_back(b);
						}
						else {
							//merge flags
							binds[b.binding].stageFlags |= b.stageFlags;
						}

					}
				}
			}
			for (auto [k, v] : binds)
			{
				ly.bindings.push_back(v);
			}
			//sort the bindings, for hash purposes
			std::sort(ly.bindings.begin(), ly.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
				return a.binding < b.binding;
				});


			ly.create_info.bindingCount = (uint32_t)ly.bindings.size();
			ly.create_info.pBindings = ly.bindings.data();
			ly.create_info.flags = 0;
			ly.create_info.pNext = 0;


			if (ly.create_info.bindingCount > 0) {
				setHashes[i] = VK_Utils::Hash_Descriptor_Layout_Info(&ly.create_info);
				vkCreateDescriptorSetLayout(device, &ly.create_info, nullptr, &setLayouts[i]);
			}
			else {
				setHashes[i] = 0;
				setLayouts[i] = VK_NULL_HANDLE;
			}
		}

		//we start from just the default empty pipeline layout info
		VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = VK_Utils::Pipeline_Layout_Create_Info();

		mesh_pipeline_layout_info.pPushConstantRanges = ConstantRanges.data();
		mesh_pipeline_layout_info.pushConstantRangeCount = (uint32_t)ConstantRanges.size();

		std::array<VkDescriptorSetLayout, 4> compactedLayouts;
		int s = 0;
		for (int i = 0; i < 4; i++) {
			if (setLayouts[i] != VK_NULL_HANDLE) {
				compactedLayouts[s] = setLayouts[i];
				s++;
			}
		}

		mesh_pipeline_layout_info.setLayoutCount = s;
		mesh_pipeline_layout_info.pSetLayouts = compactedLayouts.data();


		vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr, &BuiltLayout);

	}

	void VK_Shader::build_sets(VkDevice device, VkDescriptorPool descript_pool)
	{	
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descript_pool;
		allocInfo.descriptorSetCount = setLayouts.size();
		allocInfo.pSetLayouts = setLayouts.data();
		if (vkAllocateDescriptorSets(device, &allocInfo, sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	void VK_Shader::destroy()
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

			for (size_t i = 0; i < setLayouts.size(); i++)
			{
				if (setLayouts[i]!=VK_NULL_HANDLE)
				{
					vkDestroyDescriptorSetLayout(Device.lock()->device, setLayouts[i], nullptr);
				}
			}

			vkDestroyPipelineLayout(Device.lock()->device,BuiltLayout,nullptr);
		}

	}

	VK_Shader::~VK_Shader()
	{
		destroy();
		
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


