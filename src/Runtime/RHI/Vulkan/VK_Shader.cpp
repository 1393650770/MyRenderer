

#include"VK_Shader.h"
#include <iostream>
#include <fstream>
#include"VK_Utils.h"

#include <algorithm>
#include "spv_reflect/spirv_reflect.h"
#include "VK_Buffer.h"
#include "VK_Texture.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


VkShaderModule VK_Shader::CreateShaderModule(CONST Vector<UInt32>& code)
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

VK_Shader::VK_Shader(VK_Device* in_device, CONST ShaderDesc& desc, CONST ShaderDataPayload& data) :Shader(desc,data)
{
	device = in_device;
	shader_codes = data.data;
	shader_modules = CreateShaderModule(shader_codes);
	ReflectBindings(data.shader_binding_overrides);
}

void VK_Shader::Destroy()
{
	if (shader_modules != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device->GetDevice(), shader_modules, nullptr);
		shader_modules = VK_NULL_HANDLE;
	}
	reflect_info.bindings.clear();
	reflect_info.constant_ranges.clear();
	reflect_info.setlayouts.clear();
}

VK_Shader::~VK_Shader()
{
	Destroy();
		
}

VkShaderModule VK_Shader::GetShaderModule() CONST
{
	return shader_modules;
}

void VK_Shader::ReflectBindings(CONST Vector<ShaderDataPayload::ShaderBindingOverrides>& shader_binding_overrides)
{
	SpvReflectShaderModule spvmodule;
	SpvReflectResult result = spvReflectCreateShaderModule(shader_codes.size() * sizeof(UInt32), shader_codes.data(), &spvmodule);

	UInt32 count = 0;
	result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	Vector<SpvReflectDescriptorSet*> sets(count);
	result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (UInt32 i_set = 0; i_set < sets.size(); ++i_set)
	{

		const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

		DescriptorSetLayoutData layout = {};

		layout.bindings.resize(refl_set.binding_count);
		for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
			const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
			VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
			layout_binding.binding = refl_binding.binding;
			layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

			/*
			if (layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) 
				layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			if (layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			*/

			layout_binding.descriptorCount = 1;
			for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
				layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
			}
			layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);
			for (auto& s : shader_binding_overrides)
			{
				if (strcmp(refl_binding.name,s.name.c_str())==0)
				{
					layout_binding.descriptorType = VK_Utils::Translate_BindingResourceType_To_VulkanDescriptorType(s.overriden_type);
				}
			}
			ReflectedBinding reflected(refl_set.set, refl_binding.binding, layout_binding.descriptorType);

			reflect_info.bindings[refl_binding.name] = reflected;
			
		}
		layout.set_number = refl_set.set;
		layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout.create_info.bindingCount = refl_set.binding_count;
		layout.create_info.pBindings = layout.bindings.data();

		reflect_info.setlayouts.push_back(layout);
	}

	result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<SpvReflectBlockVariable*> pconstants(count);
	result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for(UInt32 i = 0; i < count; i++)
	{
		ReflectedConstantInfo constant_info;
		VkPushConstantRange pcs{};
		pcs.offset = pconstants[i]->offset;
		pcs.size = pconstants[i]->size;
		pcs.stageFlags = VK_Utils::Translate_ShaderTypeEnum_To_Vulkan(desc.shader_type);
		constant_info.constant = pcs;
		constant_info.name = pconstants[i]->name;
		reflect_info.constant_ranges.push_back(constant_info);
	}
}

ReflectedInfo& VK_Shader::GetReflectedInfo()
{
	return reflect_info;
}


VK_ShaderResourceBinding::~VK_ShaderResourceBinding()
{

}

VK_ShaderResourceBinding::VK_ShaderResourceBinding(VK_Device* in_device, Map<String, ReflectedBinding>& in_bindings,Bool in_is_static_resource) :device(in_device), bindings(in_bindings),is_static_resource(in_is_static_resource)
{
	
}

CONST VkDescriptorSet* VK_ShaderResourceBinding::GetDescriptorSets() CONST
{
	return descriptorset.data();
}

void VK_ShaderResourceBinding::SetResource(CONST String& name, CONST RenderResource* resource)
{
	auto it = bindings.find(name);
	if (it != bindings.end())
	{
		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		ReflectedBinding& binding = it->second;
		descriptor_write.dstSet = descriptorset[binding.set];
		descriptor_write.dstBinding = binding.binding;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = binding.type;
		descriptor_write.descriptorCount = 1;
		switch (binding.type)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		{
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = STATIC_CAST(resource, CONST VK_Buffer)->GetBuffer();
			buffer_info.offset = 0;
			buffer_info.range = VK_WHOLE_SIZE;

			descriptor_write.pBufferInfo = &buffer_info;
			break;
		}
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		{
			VkDescriptorImageInfo image_info{};
			image_info.sampler = STATIC_CAST(resource, CONST VK_Texture)->GetSampler();
			descriptor_write.pImageInfo = &image_info;
			break;
		}
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{
			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = STATIC_CAST(resource, CONST VK_Texture)->GetImageView();
			image_info.sampler = STATIC_CAST(resource, CONST VK_Texture)->GetSampler();
			descriptor_write.pImageInfo = &image_info;
			break;
		}
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		default:
			CHECK_WITH_LOG(true, "RHI Error : invalid descriptor type to SetResource");
			break;
		}

		vkUpdateDescriptorSets(device->GetDevice(), 1, &descriptor_write, 0, nullptr);
		if (is_static_resource)
		{
			bindings.erase(name);
		}
	}
	else
	{
		
		CHECK_WITH_LOG(is_static_resource==false, "RHI Error : fail to find binding name in shader");
	}
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

