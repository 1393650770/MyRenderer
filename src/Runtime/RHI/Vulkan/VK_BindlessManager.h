#pragma once
#ifndef _VK_BINDLESSMANAGER_
#define _VK_BINDLESSMANAGER_

#include <vulkan/vulkan_core.h>
#include "Core/ConstDefine.h"
#include "Core/ResourceHandle.h"
#include "RHI/RenderBindlessManager.h"

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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_BindlessManager, public MXRender::RHI::BindlessManager)
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

	// 2D texture slot management — generation-protected handles
	VIRTUAL BindlessSlotHandle METHOD(AllocateTexture2DSlot)(MXRender::RHI::Texture* texture) OVERRIDE FINAL;
	BindlessSlotHandle METHOD(AllocateTexture2DSlot)(VK_Texture* texture);
	VIRTUAL void   METHOD(FreeTexture2DSlot)(BindlessSlotHandle handle) OVERRIDE FINAL;
	void   METHOD(UpdateTexture2DSlot)(UInt32 index, VK_Texture* texture);
	void   METHOD(UpdateTexture2DSlot)(UInt32 index, MXRender::RHI::Texture* texture);

	// Cube texture slot management — generation-protected handles
	VIRTUAL BindlessCubeSlotHandle METHOD(AllocateTextureCubeSlot)(MXRender::RHI::Texture* texture) OVERRIDE FINAL;
	BindlessCubeSlotHandle METHOD(AllocateTextureCubeSlot)(VK_Texture* texture);
	VIRTUAL void   METHOD(FreeTextureCubeSlot)(BindlessCubeSlotHandle handle) OVERRIDE FINAL;

	// Global shared descriptor set (all pipelines referencing Set 3 share this)
	VkDescriptorSetLayout METHOD(GetLayout)() CONST;
	VkDescriptorSet METHOD(GetDescriptorSet)() CONST;
	VIRTUAL Bool METHOD(IsEnabled)() CONST OVERRIDE FINAL;

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

	// 2D slot management — generation-protected sparse array + free list
	struct SlotEntry2D
	{
		UInt32 generation = 1;
		UInt32 next_free = 0;  // free-list pointer, 0 = no next
	};
	Vector<SlotEntry2D> slot_meta_2d;
	UInt32 free_head_2d = 0;

	// Cube slot management — generation-protected sparse array + free list
	struct SlotEntryCube
	{
		UInt32 generation = 1;
		UInt32 next_free = 0;
	};
	Vector<SlotEntryCube> slot_meta_cube;
	UInt32 free_head_cube = 0;

private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // _VK_BINDLESSMANAGER_
