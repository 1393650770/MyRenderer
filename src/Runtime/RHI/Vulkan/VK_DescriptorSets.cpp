
#include "VK_DescriptorSets.h"
#include <iostream>
#include <fstream>
#include "VK_Device.h"
#include <algorithm>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)



VK_DescriptorPoolManager::~VK_DescriptorPoolManager()
{
	
}

VK_DescriptorPoolManager::VK_DescriptorPoolManager(VK_Device* in_device, UInt32 in_max_descriptorsets) : device(in_device), max_descriptorsets(in_max_descriptorsets)
{
	descriptor_pool = CreatePool();
}

VkDescriptorPool VK_DescriptorPoolManager::CreatePool()
{
	CONST UInt32 limit_max_uniform_buffers = max_descriptorsets * 2;
	CONST UInt32 limit_max_samplers = max_descriptorsets / 2;
	CONST UInt32 limit_max_combined_image_samplers = max_descriptorsets * 3;
	CONST UInt32 limit_max_uniform_texel_buffers = max_descriptorsets / 2;
	CONST UInt32 limit_max_storage_texel_buffers = max_descriptorsets / 4;
	CONST UInt32 limit_max_storage_buffers = max_descriptorsets / 4;
	CONST UInt32 limit_max_storage_image = max_descriptorsets / 4;
	CONST UInt32 limit_max_sampled_images = max_descriptorsets * 2;
	CONST UInt32 limit_max_input_attachments = max_descriptorsets / 16;

	std::vector<VkDescriptorPoolSize> types(10);
	types[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	types[0].descriptorCount = limit_max_uniform_buffers;

	types[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	types[1].descriptorCount = limit_max_uniform_buffers;


	types[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	types[2].descriptorCount = limit_max_samplers;


	types[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	types[3].descriptorCount = limit_max_combined_image_samplers;


	types[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	types[4].descriptorCount = limit_max_sampled_images;


	types[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	types[5].descriptorCount = limit_max_uniform_texel_buffers;


	types[6].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	types[6].descriptorCount = limit_max_storage_texel_buffers;


	types[7].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	types[7].descriptorCount = limit_max_storage_buffers;


	types[8].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	types[8].descriptorCount = limit_max_storage_image;

	types[9].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	types[9].descriptorCount = limit_max_input_attachments;


	VkDescriptorPoolCreateInfo pool_info;
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.poolSizeCount = types.size();
	pool_info.pPoolSizes = types.data();
	pool_info.maxSets = max_descriptorsets;
	pool_info.pNext = 0U;
	VkDescriptorPool temp_descriptor_pool;
	CHECK_WITH_LOG(vkCreateDescriptorPool(device->GetDevice(), &pool_info, nullptr, &temp_descriptor_pool) != VK_SUCCESS,
		("failed to create descriptor pool!"));
	
	return temp_descriptor_pool;

}

VkDescriptorPool VK_DescriptorPoolManager::GrabPool()
{
	if (free_pools.size() > 0)
	{
		VkDescriptorPool pool = free_pools.back();
		free_pools.pop_back();
		return pool;
	}

	return CreatePool();
	
}

UInt32 VK_DescriptorPoolManager::GetMaxDescriptorsets() CONST
{
	return max_descriptorsets;
}

CONST VkDescriptorPool& VK_DescriptorPoolManager::GetDescriptorPool()
{
	return descriptor_pool;
}

Bool VK_DescriptorPoolManager::AllocateDescriptorset(VkDescriptorSetLayout layout, VkDescriptorSet& out_set)
{
	if (descriptor_pool == VK_NULL_HANDLE)
	{
		descriptor_pool = GrabPool();
		used_pools.push_back(descriptor_pool);
	}

	VkDescriptorSetAllocateInfo descriptor_set_allocateInfo;
	descriptor_set_allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocateInfo.descriptorPool = descriptor_pool;
	descriptor_set_allocateInfo.descriptorSetCount = 1;
	descriptor_set_allocateInfo.pSetLayouts = &layout;
	descriptor_set_allocateInfo.pNext = VK_NULL_HANDLE;
	VkResult alloc_result = vkAllocateDescriptorSets(device->GetDevice(), &descriptor_set_allocateInfo, &out_set);
	bool need_reallocate = false;

	switch (alloc_result) 
	{
	case VK_SUCCESS:
		//all good, return
		already_create_set[descriptor_pool].push_back(out_set);
		return true;

		break;
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		//reallocate pool
		need_reallocate = true;
		break;
	default:
		//unrecoverable error
		need_reallocate = true;
		return false;
	}

	if (need_reallocate)
	{
		//allocate a new pool and retry
		descriptor_pool = GrabPool();
		used_pools.push_back(descriptor_pool);

		alloc_result = vkAllocateDescriptorSets(device->GetDevice(), &descriptor_set_allocateInfo, &out_set);

		if (alloc_result == VK_SUCCESS)
		{
			already_create_set[descriptor_pool].push_back(out_set);
			return true;
		}
	}
	return false;
}

Bool VK_DescriptorPoolManager::AllocateDescriptorsets(CONST VkDescriptorSetAllocateInfo& in_descriptorset_allocate_info, VkDescriptorSet& out_sets)
{
	if (descriptor_pool == VK_NULL_HANDLE)
	{
		descriptor_pool = GrabPool();
		used_pools.push_back(descriptor_pool);
	}

	VkDescriptorSetAllocateInfo descriptor_set_allocateInfo = in_descriptorset_allocate_info;
	descriptor_set_allocateInfo.descriptorPool = descriptor_pool;
	VkResult alloc_result = vkAllocateDescriptorSets(device->GetDevice(), &descriptor_set_allocateInfo, &out_sets);
	Bool need_reallocate = false;

	switch (alloc_result) 
	{
	case VK_SUCCESS:
		already_create_set[descriptor_pool].push_back(out_sets);
		return true;

		break;
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		//reallocate pool
		need_reallocate = true;
		break;
	default:
		//unrecoverable error
		return false;
	}

	if (need_reallocate)
	{
		//allocate a new pool and retry
		descriptor_pool = GrabPool();
		used_pools.push_back(descriptor_pool);

		alloc_result = vkAllocateDescriptorSets(device->GetDevice(), &descriptor_set_allocateInfo, &out_sets);

		if (alloc_result == VK_SUCCESS)
		{
			already_create_set[descriptor_pool].push_back(out_sets);
			return true;
		}
	}
	return false;
}

void VK_DescriptorPoolManager::ResetDescriptPool(VkDescriptorPoolResetFlags flags)
{
	for (auto p : used_pools)
	{
		vkResetDescriptorPool(device->GetDevice(), p, flags);
	}

	free_pools = used_pools;
	used_pools.clear();
	descriptor_pool = VK_NULL_HANDLE;
}

VK_DescriptsetAllocator::~VK_DescriptsetAllocator()
{

}

VK_DescriptsetAllocator::VK_DescriptsetAllocator(VK_Device* in_device, UInt32 in_max_descriptorsets) : VK_DescriptorPoolManager(in_device, in_max_descriptorsets)
{
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

