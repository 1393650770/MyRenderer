#pragma once
#ifndef _VK_BINDLESSMANAGER_
#define _VK_BINDLESSMANAGER_

#include <vulkan/vulkan_core.h>
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;
class VK_Texture;

MYRENDERER_BEGIN_CLASS(VK_BindlessManager)
#pragma region METHOD
public:
	static constexpr UInt32 BINDLESS_SET_INDEX = 2;
	static constexpr UInt32 BINDLESS_TEXTURE_2D_BINDING = 0;
	static constexpr UInt32 BINDLESS_TEXTURE_CUBE_BINDING = 1;
	static constexpr UInt32 BINDLESS_SAMPLER_BINDING = 2;
	static constexpr UInt32 MAX_TEXTURES_2D = 4096;
	static constexpr UInt32 MAX_TEXTURES_CUBE = 256;
	static constexpr UInt32 MAX_SAMPLERS = 64;

	VK_BindlessManager(VK_Device* in_device);
	~VK_BindlessManager();

	// 2D texture slot management
	UInt32 METHOD(AllocateTexture2DSlot)(MXRender::RHI::Texture* texture);
	UInt32 METHOD(AllocateTexture2DSlot)(VK_Texture* texture);
	void   METHOD(FreeTexture2DSlot)(UInt32 index);
	void   METHOD(UpdateTexture2DSlot)(UInt32 index, VK_Texture* texture);
	void   METHOD(UpdateTexture2DSlot)(UInt32 index, MXRender::RHI::Texture* texture);

	// Cube texture slot management
	UInt32 METHOD(AllocateTextureCubeSlot)(MXRender::RHI::Texture* texture);
	UInt32 METHOD(AllocateTextureCubeSlot)(VK_Texture* texture);
	void   METHOD(FreeTextureCubeSlot)(UInt32 index);

	// Global shared descriptor set (all pipelines referencing Set 3 share this)
	VkDescriptorSetLayout METHOD(GetLayout)() CONST;
	VkDescriptorSet METHOD(GetDescriptorSet)() CONST;
	Bool METHOD(IsEnabled)() CONST;

protected:
	void METHOD(CreateBindlessPool)();
	void METHOD(CreateBindlessLayout)();
	void METHOD(CreateBindlessDescriptorSet)();

private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	VK_Device* device;
	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	Bool is_enabled = false;

	// 2D slot management
	Vector<UInt32> free_slots_2d;
	Vector<Bool>   slot_used_2d;
	UInt32 next_free_index_2d = 0;

	// Cube slot management
	Vector<UInt32> free_slots_cube;
	Vector<Bool>   slot_used_cube;
	UInt32 next_free_index_cube = 0;

private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // _VK_BINDLESSMANAGER_
