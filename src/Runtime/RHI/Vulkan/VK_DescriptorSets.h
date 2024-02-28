#pragma once
#ifndef _VK_DESCRIPTORSETS_
#define _VK_DESCRIPTORSETS_
#include<vulkan/vulkan.h>
#include <string>
#include<vector>
#include<memory>
#include<unordered_map> 
#include"glm/glm.hpp"
#include "RHI/RenderEnum.h"
#include "RHI/RenderShader.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
class VK_Device;
class VK_DescriptorPoolManager;
class VK_DescriptorSetLayout;



MYRENDERER_BEGIN_CLASS(VK_DescriptorPoolManager)
#pragma region METHOD
public:

    VK_DescriptorPoolManager(VK_Device* in_device, UInt32 in_max_descriptorsets);

	VIRTUAL ~VK_DescriptorPoolManager();

    UInt32 METHOD(GetMaxDescriptorsets)() CONST;
	CONST VkDescriptorPool& METHOD(GetDescriptorPool)();
	Bool METHOD(AllocateDescriptorset)(VkDescriptorSetLayout layout, VkDescriptorSet& out_set);
    Bool METHOD(AllocateDescriptorsets)(CONST VkDescriptorSetAllocateInfo& in_descriptorset_allocate_info, VkDescriptorSet& out_sets);
	void METHOD(ResetDescriptPool)(VkDescriptorPoolResetFlags flags);
protected:
	VkDescriptorPool METHOD(CreatePool)();
	VkDescriptorPool METHOD(GrabPool)();
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	VK_Device* device;
	Map<VkDescriptorPool, Vector<VkDescriptorSet>> already_create_set;
	UInt32 max_descriptorsets = 0;
	VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
	Vector<VkDescriptorPool> used_pools;
	Vector<VkDescriptorPool> free_pools;

private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_DescriptsetAllocator,public VK_DescriptorPoolManager)
#pragma region METHOD
public:

    VK_DescriptsetAllocator(VK_Device* in_device, UInt32 in_max_descriptorsets);

	VIRTUAL ~VK_DescriptsetAllocator();


protected:

private:

#pragma endregion

#pragma region MEMBER
public:
protected:

private:
#pragma endregion
MYRENDERER_END_CLASS




MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif //_VK_DESCRIPTORSETS_
