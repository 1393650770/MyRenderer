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

VK_TextureView() DEFAULT;

void METHOD(Create)(VK_Device& device, VkImage in_image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, ENUM_TEXTURE_FORMAT vk_format, VkFormat format, UInt32 first_mip, UInt32 num_mips, UInt32 array_slice_index, UInt32 num_array_slices, Bool use_identity_swizzle = false);

void METHOD(Destroy)(VK_Device& device);

VkImageView view=VK_NULL_HANDLE;
VkImage image=VK_NULL_HANDLE;
UInt32 viewId=0;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Texture,public Texture)
   
#pragma region METHOD
public:
	virtual ~VK_Texture();
	VK_Texture(VK_Device& in_device, const TextureDesc& texture_desc);

	static void METHOD(GenerateImageCreateInfo)(VkImageCreateInfo& image_create_info, VK_Device& in_device, const TextureDesc& desc);
	static void METHOD(GenerateViewCreateInfo)(VkImageViewCreateInfo& view_create_info, VK_Device& in_device, const TextureDesc& desc, VkImage& texture_image);
	
	void METHOD(UpdateTextureData)(CONST TextureDataPayload& texture_data_payload) OVERRIDE;

protected:
	void load_dds(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
	void load_dds_cubemap(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
	void load_dds_2d(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
	void load_common_2d(const std::string& texture_path);
	VkFormat trans_gli_format_to_vulkan(gli::format format);
	std::string get_file_extension(const std::string& filename);

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	VK_Device* device;
	VkDeviceSize imageSize;

	VkImage texture_image;
	VkImageView texture_image_view;
	VkSampler texture_sampler;

	VkImageLayout texture_image_layout;

	VK_Allocation allocation;

	VkImageCreateInfo image_create_info;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif