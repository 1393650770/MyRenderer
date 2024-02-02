

#include"VK_Shader.h"
#include <iostream>
#include <fstream>
#include"VK_Utils.h"

#include <algorithm>
#include "spv_reflect/spirv_reflect.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


Vector<UInt32> VK_Shader::read_file(CONST String& file_name)
{
	std::ifstream file(file_name, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	UInt32 fileSize = (UInt32)file.tellg();
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));

	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);

	file.close();

	return buffer;
}


VkShaderModule VK_Shader::create_shader_module(CONST Vector<UInt32>& code)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	create_info.codeSize = code.size() * sizeof(UInt32);
	create_info.pCode = (code.data());
	VkShaderModule shader_module;


	if (vkCreateShaderModule(device->GetDevice(), &create_info, nullptr, &shader_module) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}

std::tuple<VkDeviceSize, VkBuffer, VkDeviceMemory>& VK_Shader::getUniformTuple(CONST String& name)
{
	return uniform_map[name];
}


VK_Shader::VK_Shader(VK_Device* in_device, Vector<ShaderData>& in_shader_data)
{
	device = in_device;

	for(auto& shader_data:in_shader_data)
	{
		shader_codes[shader_data.stage] =std::move(shader_data.spirv);
		shader_modules[shader_data.stage] = create_shader_module(shader_codes[shader_data.stage]);
		shader_data.spirv.clear();
	}
	
}

VkPipelineLayout VK_Shader::get_built_layout()
{
	return built_layout;
}

void VK_Shader::fill_stages(Vector<VkPipelineShaderStageCreateInfo>& pipeline_stages)
{
	for (int index = 0; index < ENUM_SHADER_STAGE::NumStages; index++)
	{
		if (shader_modules[index] != VK_NULL_HANDLE)
		{
			pipeline_stages.push_back(VK_Utils::Pipeline_Shader_Stage_Create_Info(VK_Utils::Translate_API_ShaderTypeEnum_To_Vulkan((ENUM_SHADER_STAGE)index), shader_modules[index]));
		}
	}
}

void VK_Shader::reflect_layout(ReflectionOverrides* overrides, int override_count)
{
	Vector<DescriptorSetLayoutData> set_layout_datas;
	Vector<VkPushConstantRange> constant_ranges;
	for (int index = 0 ;index< ENUM_SHADER_STAGE::NumStages;index++)
	{
		if (shader_modules[index]!=VK_NULL_HANDLE)
		{
			SpvReflectShaderModule spvmodule;
			SpvReflectResult result = spvReflectCreateShaderModule(shader_codes[index].size() * sizeof(UInt32), shader_codes[index].data(), &spvmodule);

			UInt32 count = 0;
			result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			Vector<SpvReflectDescriptorSet*> sets(count);
			result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			for (UInt32 i_set = 0; i_set < sets.size(); ++i_set) 
			{

				CONST SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

				DescriptorSetLayoutData layout = {};

				layout.bindings.resize(refl_set.binding_count);
				for (UInt32 i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
					CONST SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
					VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
					layout_binding.binding = refl_binding.binding;
					layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

					for (int ov = 0; ov < override_count; ov++)
					{
						if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
							layout_binding.descriptorType = overrides[ov].override_type;
						}
					}

					layout_binding.descriptorCount = 1;
					for (UInt32 i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
						layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
					}
					layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);

					ReflectedBinding reflected;
					reflected.binding = layout_binding.binding;
					reflected.set = refl_set.set;
					reflected.type = layout_binding.descriptorType;

					bindings[refl_binding.name] = reflected;
				}
				layout.set_number = refl_set.set;
				layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout.create_info.bindingCount = refl_set.binding_count;
				layout.create_info.pBindings = layout.bindings.data();

				set_layout_datas.push_back(layout);
			}

			result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			Vector<SpvReflectBlockVariable*> pconstants(count);
			result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			if (count > 0) {
				VkPushConstantRange pcs{};
				pcs.offset = pconstants[0]->offset;
				pcs.size = pconstants[0]->size;
				pcs.stageFlags = VK_Utils::Translate_API_ShaderTypeEnum_To_Vulkan((ENUM_SHADER_STAGE)index);

				constant_ranges.push_back(pcs);
			}
		}
		
	}


	std::array<DescriptorSetLayoutData, 4> merged_layouts;

	for (int i = 0; i < 4; i++) {

		DescriptorSetLayoutData& ly = merged_layouts[i];

		ly.set_number = i;

		ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		std::unordered_map<int, VkDescriptorSetLayoutBinding> binds;
		for (auto& s : set_layout_datas) {
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


		ly.create_info.bindingCount = (UInt32)ly.bindings.size();
		ly.create_info.pBindings = ly.bindings.data();
		ly.create_info.flags = 0;
		ly.create_info.pNext = 0;


		if (ly.create_info.bindingCount > 0) {
			set_hashes[i] = VK_Utils::Hash_Descriptor_Layout_Info(&ly.create_info);
			vkCreateDescriptorSetLayout(device->GetDevice(), &ly.create_info, nullptr, &set_layouts[i]);
		}
		else {
			set_hashes[i] = 0;
			set_layouts[i] = VK_NULL_HANDLE;
		}
	}

	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = VK_Utils::Pipeline_Layout_Create_Info();

	mesh_pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
	mesh_pipeline_layout_info.pushConstantRangeCount = (UInt32)constant_ranges.size();

	std::array<VkDescriptorSetLayout, 4> compactedLayouts;
	int s = 0;
	for (int i = 0; i < 4; i++) {
		if (set_layouts[i] != VK_NULL_HANDLE) {
			compactedLayouts[s] = set_layouts[i];
			s++;
		}
	}

	mesh_pipeline_layout_info.setLayoutCount = s;
	mesh_pipeline_layout_info.pSetLayouts = compactedLayouts.data();


	vkCreatePipelineLayout(device->GetDevice(), &mesh_pipeline_layout_info, nullptr, &built_layout);

}

void VK_Shader::build_sets(VkDevice device, VkDescriptorPool descript_pool)
{	
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descript_pool;
	alloc_info.descriptorSetCount = set_layouts.size();
	alloc_info.pSetLayouts = set_layouts.data();
	if (vkAllocateDescriptorSets(device, &alloc_info, sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

void VK_Shader::destroy()
{

	for (UInt32 i = 0; i < ENUM_SHADER_STAGE::NumStages; i++)
	{
		if (shader_modules[i] != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(device->GetDevice(), shader_modules[i], nullptr);
			shader_modules[i]=VK_NULL_HANDLE;
		}
	}

	for (UInt32 i = 0; i < set_layouts.size(); i++)
	{
		if (set_layouts[i] != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(device->GetDevice(), set_layouts[i], nullptr);
			set_layouts[i] = VK_NULL_HANDLE;
		}
	}

	vkDestroyPipelineLayout(device->GetDevice(), built_layout, nullptr);


}

VK_Shader::~VK_Shader()
{
	destroy();
		
}





MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

