#include "VK_BindlessManager.h"
#include "VK_Device.h"
#include "VK_Texture.h"
#include "RHI/RenderTexture.h"
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

VK_BindlessManager::VK_BindlessManager(VK_Device* in_device)
	: device(in_device)
{
	if (!device->GetOptionalExtensions().HasEXTDescriptorIndexing)
	{
		return; // Bindless not supported on this device
	}

	// Initialize slot tracking
	slot_used_2d.resize(MAX_TEXTURES_2D, false);
	free_slots_2d.reserve(MAX_TEXTURES_2D);
	slot_used_cube.resize(MAX_TEXTURES_CUBE, false);
	free_slots_cube.reserve(MAX_TEXTURES_CUBE);

	CreateBindlessPool();
	CreateBindlessLayout();
	CreateBindlessDescriptorSet();

	is_enabled = true;
}

VK_BindlessManager::~VK_BindlessManager()
{
	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device->GetDevice(), layout, nullptr);
		layout = VK_NULL_HANDLE;
	}
	if (pool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device->GetDevice(), pool, nullptr);
		pool = VK_NULL_HANDLE;
	}
	// descriptor_set is implicitly freed with pool
	descriptor_set = VK_NULL_HANDLE;
}

void VK_BindlessManager::CreateBindlessPool()
{
	VkDescriptorPoolSize pool_sizes[3] = {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[0].descriptorCount = MAX_TEXTURES_2D + MAX_TEXTURES_CUBE;
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;  // for cube
	pool_sizes[1].descriptorCount = MAX_TEXTURES_CUBE;
	pool_sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	pool_sizes[2].descriptorCount = MAX_SAMPLERS;

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	pool_info.maxSets = 1;  // Single global descriptor set
	pool_info.poolSizeCount = 3;
	pool_info.pPoolSizes = pool_sizes;

	CHECK_WITH_LOG(
		vkCreateDescriptorPool(device->GetDevice(), &pool_info, nullptr, &pool) != VK_SUCCESS,
		"RHI Error: Failed to create bindless descriptor pool");
}

void VK_BindlessManager::CreateBindlessLayout()
{
	VkDescriptorSetLayoutBinding bindings[3] = {};

	// Binding 0: 2D texture array (variable count, update-after-bind)
	bindings[0].binding = BINDLESS_TEXTURE_2D_BINDING;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].descriptorCount = MAX_TEXTURES_2D;
	bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
	bindings[0].pImmutableSamplers = nullptr;

	// Binding 1: Cube texture array (variable count, update-after-bind)
	bindings[1].binding = BINDLESS_TEXTURE_CUBE_BINDING;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].descriptorCount = MAX_TEXTURES_CUBE;
	bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
	bindings[1].pImmutableSamplers = nullptr;

	// Binding 2: Sampler array (variable count, update-after-bind)
	bindings[2].binding = BINDLESS_SAMPLER_BINDING;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	bindings[2].descriptorCount = MAX_SAMPLERS;
	bindings[2].stageFlags = VK_SHADER_STAGE_ALL;
	bindings[2].pImmutableSamplers = nullptr;

	// Per-binding flags for bindless (update-after-bind + partially bound)
	// NOTE: VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT is NOT used here
	// because it can only be set on the last binding (highest binding number).
	// We allocate the full fixed-size arrays (4096+256+64), so variable count is unnecessary.
	VkDescriptorBindingFlags binding_flags[3] = {};
	binding_flags[0] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		| VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	binding_flags[1] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		| VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	binding_flags[2] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		| VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
	binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	binding_flags_info.bindingCount = 3;
	binding_flags_info.pBindingFlags = binding_flags;

	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.pNext = &binding_flags_info;
	layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	layout_info.bindingCount = 3;
	layout_info.pBindings = bindings;

	CHECK_WITH_LOG(
		vkCreateDescriptorSetLayout(device->GetDevice(), &layout_info, nullptr, &layout) != VK_SUCCESS,
		"RHI Error: Failed to create bindless descriptor set layout");
}

void VK_BindlessManager::CreateBindlessDescriptorSet()
{
	// Allocate the descriptor set with fixed-size arrays (no variable descriptor count needed)

	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.descriptorPool = pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &layout;

	CHECK_WITH_LOG(
		vkAllocateDescriptorSets(device->GetDevice(), &alloc_info, &descriptor_set) != VK_SUCCESS,
		"RHI Error: Failed to allocate bindless descriptor set");
}

// RHI::Texture* overloads — cast to VK_Texture internally
UInt32 VK_BindlessManager::AllocateTexture2DSlot(MXRender::RHI::Texture* in_texture)
{
	return AllocateTexture2DSlot(static_cast<VK_Texture*>(in_texture));
}

UInt32 VK_BindlessManager::AllocateTextureCubeSlot(MXRender::RHI::Texture* in_texture)
{
	return AllocateTextureCubeSlot(static_cast<VK_Texture*>(in_texture));
}

void VK_BindlessManager::UpdateTexture2DSlot(UInt32 index, MXRender::RHI::Texture* in_texture)
{
	UpdateTexture2DSlot(index, static_cast<VK_Texture*>(in_texture));
}

UInt32 VK_BindlessManager::AllocateTexture2DSlot(VK_Texture* texture)
{
	if (!is_enabled || !texture) return 0;

	UInt32 index;
	if (!free_slots_2d.empty())
	{
		index = free_slots_2d.back();
		free_slots_2d.pop_back();
	}
	else
	{
		index = next_free_index_2d++;
		CHECK_WITH_LOG(index >= MAX_TEXTURES_2D, "RHI Error: Bindless 2D texture slots exhausted!");
	}

	slot_used_2d[index] = true;
	UpdateTexture2DSlot(index, texture);
	return index;
}

void VK_BindlessManager::FreeTexture2DSlot(UInt32 index)
{
	if (!is_enabled || index >= MAX_TEXTURES_2D) return;
	if (!slot_used_2d[index]) return;

	slot_used_2d[index] = false;
	free_slots_2d.push_back(index);
}

void VK_BindlessManager::UpdateTexture2DSlot(UInt32 index, VK_Texture* texture)
{
	if (!is_enabled || index >= MAX_TEXTURES_2D || !texture) return;

	VkDescriptorImageInfo image_info{};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = texture->GetImageView();
	image_info.sampler = texture->GetSampler();

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptor_set;
	write.dstBinding = BINDLESS_TEXTURE_2D_BINDING;
	write.dstArrayElement = index;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &image_info;

	vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
}

UInt32 VK_BindlessManager::AllocateTextureCubeSlot(VK_Texture* texture)
{
	if (!is_enabled || !texture) return 0;

	UInt32 index;
	if (!free_slots_cube.empty())
	{
		index = free_slots_cube.back();
		free_slots_cube.pop_back();
	}
	else
	{
		index = next_free_index_cube++;
		CHECK_WITH_LOG(index >= MAX_TEXTURES_CUBE, "RHI Error: Bindless cube texture slots exhausted!");
	}

	slot_used_cube[index] = true;

	VkDescriptorImageInfo image_info{};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = texture->GetImageView();
	image_info.sampler = texture->GetSampler();

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptor_set;
	write.dstBinding = BINDLESS_TEXTURE_CUBE_BINDING;
	write.dstArrayElement = index;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.pImageInfo = &image_info;

	vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);

	return index;
}

void VK_BindlessManager::FreeTextureCubeSlot(UInt32 index)
{
	if (!is_enabled || index >= MAX_TEXTURES_CUBE) return;
	if (!slot_used_cube[index]) return;

	slot_used_cube[index] = false;
	free_slots_cube.push_back(index);
}

VkDescriptorSetLayout VK_BindlessManager::GetLayout() CONST
{
	return layout;
}

VkDescriptorSet VK_BindlessManager::GetDescriptorSet() CONST
{
	return descriptor_set;
}

Bool VK_BindlessManager::IsEnabled() CONST
{
	return is_enabled;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
