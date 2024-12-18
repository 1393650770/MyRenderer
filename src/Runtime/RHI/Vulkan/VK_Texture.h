#pragma once

#ifndef _VK_TEXTURE_
#define _VK_TEXTURE_

#include"RHI/RenderEnum.h"

#include <vector>
#include <string>

#include "RHI/RenderTexture.h"
#include "vulkan/vulkan_core.h"
#include "gli/format.hpp"
#include "VK_Device.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_STRUCT(VK_TextureView)

VK_TextureView() MYDEFAULT;

void METHOD(Create)(VK_Device& device, VkImage in_image, VkImageViewType view_type, VkImageAspectFlags aspect_flags,VkFormat format, UInt32 first_mip, UInt32 num_mips, UInt32 array_slice_index, UInt32 num_array_slices, Bool use_identity_swizzle = false);

void METHOD(Destroy)(VK_Device& device);

VkImageView view=VK_NULL_HANDLE;
VkImage image=VK_NULL_HANDLE;
UInt32 viewId=0;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Texture, public Texture)
friend class VK_Viewport;
#pragma region METHOD
public:
	virtual ~VK_Texture();
	VK_Texture(VK_Device* in_device, CONST TextureDesc& texture_desc);
	VK_Texture(VK_Device* in_device, CONST VK_TextureView& texture_view, CONST TextureDesc& texture_desc);
	static void METHOD(GenerateImageCreateInfo)(VkImageCreateInfo& image_create_info, VK_Device& in_device, CONST TextureDesc& desc);
	static void METHOD(GenerateViewCreateInfo)(VkImageViewCreateInfo& view_create_info, VK_Device& in_device, CONST TextureDesc& desc, VkImage& texture_image);
	
	void METHOD(UpdateTextureData)(CONST TextureDataPayload& texture_data_payload) OVERRIDE;
	VkImage METHOD(GetImage)() CONST;
	VkImageView METHOD(GetImageView)() CONST;
	VkSampler METHOD(GetSampler)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VK_Device* device;
	VkDeviceSize imageSize;

	VkImage texture_image = VK_NULL_HANDLE;
	VkImageView texture_image_view = VK_NULL_HANDLE;
	VkSampler texture_sampler = VK_NULL_HANDLE;

	VkImageLayout texture_image_layout;

	VK_Allocation allocation;
	Bool is_proxy_texture = false;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif