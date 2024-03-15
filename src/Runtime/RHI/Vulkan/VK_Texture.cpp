#include"VK_Texture.h"
#include "Core/ConstDefine.h"
#include "VK_Define.h"
#include "VK_Utils.h"
#include "VK_Buffer.h"
#include "VK_CommandBuffer.h"
#include "VK_Queue.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

void VK_TextureView::Create(VK_Device& device, VkImage in_image, VkImageViewType view_type, VkImageAspectFlags aspect_flags, VkFormat format, UInt32 first_mip, UInt32 num_mips, UInt32 array_slice_index, UInt32 num_array_slices, Bool use_identity_swizzle)
{
	image = in_image;

	VkImageViewCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.viewType = view_type;
	create_info.format = format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.subresourceRange.aspectMask = aspect_flags;
	create_info.subresourceRange.baseMipLevel = first_mip;
	create_info.subresourceRange.levelCount = num_mips;
	create_info.subresourceRange.baseArrayLayer = array_slice_index;
	create_info.subresourceRange.layerCount = num_array_slices;

	if (vkCreateImageView(device.GetDevice(), &create_info, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image views!");
	}



}

void VK_TextureView::Destroy(VK_Device& device)
{
	if (view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device.GetDevice(), view, nullptr);
		view = VK_NULL_HANDLE;
	}
}



VK_Texture::VK_Texture(VK_Device* in_device, CONST TextureDesc& texture_desc):Texture(texture_desc),device(in_device)
{
	GenerateImageCreateInfo(image_create_info, *in_device, texture_desc);

	CHECK_WITH_LOG(vkCreateImage(in_device->GetDevice(), &image_create_info, VULKAN_CPU_ALLOCATOR, &texture_image) != VK_SUCCESS,
					"RHI Error: failed to CreateImage !")
	texture_image_layout = image_create_info.initialLayout;

	CONST ENUM_VulkanAllocationFlags alloc_flags = ENUM_VulkanAllocationFlags::AutoBind ;

	in_device->GetMemoryManager()->AllocateImageMemory(allocation,texture_image, alloc_flags,0);

	allocation.BindImage(in_device,texture_image);

	VkImageViewCreateInfo image_view_create_info;
	GenerateViewCreateInfo(image_view_create_info, *in_device, texture_desc,texture_image);

	CHECK_WITH_LOG(vkCreateImageView(in_device->GetDevice(), &image_view_create_info, VULKAN_CPU_ALLOCATOR, &texture_image_view) != VK_SUCCESS,
		"RHI Error: failed to CreateImageView !")

	//texture_image_layout = VK_Utils::Translate_Texture_usage_type_To_Vulkan(texture_desc.usage);

	texture_sampler = VK_Utils::Create_Linear_Sampler(in_device->GetGpu(), in_device->GetDevice());
}

VK_Texture::VK_Texture(VK_Device* in_device, CONST VK_TextureView& texture_view, CONST TextureDesc& texture_desc) :Texture(texture_desc),device(in_device)
{
	texture_image = texture_view.image;
	texture_image_view = texture_view.view;
	imageSize = texture_desc.width * texture_desc.height * 4;
	texture_image_layout = VK_Utils::Translate_Texture_usage_type_To_Vulkan(texture_desc.usage);
	//texture_sampler = VK_Utils::Create_Linear_Sampler(in_device->GetGpu(), in_device->GetDevice());
}

VkFormat VK_Texture::trans_gli_format_to_vulkan(gli::format format)
{
	switch (format)
	{
	case gli::FORMAT_RGBA8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case gli::FORMAT_RGB8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8_UNORM;
	case gli::FORMAT_RGBA16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case gli::FORMAT_RGBA32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case gli::FORMAT_R8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::FORMAT_R16_SFLOAT_PACK16:
		return VK_FORMAT_R16_SFLOAT;
	case gli::FORMAT_R32_SFLOAT_PACK32:
		return VK_FORMAT_R32_SFLOAT;
	case gli::FORMAT_RG8_UNORM_PACK8:
		return VK_FORMAT_R8G8_UNORM;
	case gli::FORMAT_RG16_SFLOAT_PACK16:
		return VK_FORMAT_R16G16_SFLOAT;
	case gli::FORMAT_RG32_SFLOAT_PACK32:
		return VK_FORMAT_R32G32_SFLOAT;
	case gli::FORMAT_RGB_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case gli::FORMAT_UNDEFINED:
		return VK_FORMAT_UNDEFINED;
		break;
	case gli::FORMAT_RG4_UNORM_PACK8:
		return VK_FORMAT_R4G4_UNORM_PACK8;
		break;
	case gli::FORMAT_RGBA4_UNORM_PACK16:
		break;
	case gli::FORMAT_BGRA4_UNORM_PACK16:
		break;
	case gli::FORMAT_R5G6B5_UNORM_PACK16:
		break;
	case gli::FORMAT_B5G6R5_UNORM_PACK16:
		break;
	case gli::FORMAT_RGB5A1_UNORM_PACK16:
		break;
	case gli::FORMAT_BGR5A1_UNORM_PACK16:
		break;
	case gli::FORMAT_A1RGB5_UNORM_PACK16:
		break;
	case gli::FORMAT_R8_SNORM_PACK8:
		break;
	case gli::FORMAT_R8_USCALED_PACK8:
		break;
	case gli::FORMAT_R8_SSCALED_PACK8:
		break;
	case gli::FORMAT_R8_UINT_PACK8:
		break;
	case gli::FORMAT_R8_SINT_PACK8:
		break;
	case gli::FORMAT_R8_SRGB_PACK8:
		break;
	case gli::FORMAT_RG8_SNORM_PACK8:
		break;
	case gli::FORMAT_RG8_USCALED_PACK8:
		break;
	case gli::FORMAT_RG8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RG8_UINT_PACK8:
		break;
	case gli::FORMAT_RG8_SINT_PACK8:
		break;
	case gli::FORMAT_RG8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGB8_SNORM_PACK8:
		break;
	case gli::FORMAT_RGB8_USCALED_PACK8:
		break;
	case gli::FORMAT_RGB8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RGB8_UINT_PACK8:
		break;
	case gli::FORMAT_RGB8_SINT_PACK8:
		break;
	case gli::FORMAT_RGB8_SRGB_PACK8:
		break;
	case gli::FORMAT_BGR8_UNORM_PACK8:
		break;
	case gli::FORMAT_BGR8_SNORM_PACK8:
		break;
	case gli::FORMAT_BGR8_USCALED_PACK8:
		break;
	case gli::FORMAT_BGR8_SSCALED_PACK8:
		break;
	case gli::FORMAT_BGR8_UINT_PACK8:
		break;
	case gli::FORMAT_BGR8_SINT_PACK8:
		break;
	case gli::FORMAT_BGR8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGBA8_SNORM_PACK8:
		break;
	case gli::FORMAT_RGBA8_USCALED_PACK8:
		break;
	case gli::FORMAT_RGBA8_SSCALED_PACK8:
		break;
	case gli::FORMAT_RGBA8_UINT_PACK8:
		break;
	case gli::FORMAT_RGBA8_SINT_PACK8:
		break;
	case gli::FORMAT_RGBA8_SRGB_PACK8:
		break;
	case gli::FORMAT_BGRA8_UNORM_PACK8:
		break;
	case gli::FORMAT_BGRA8_SNORM_PACK8:
		break;
	case gli::FORMAT_BGRA8_USCALED_PACK8:
		break;
	case gli::FORMAT_BGRA8_SSCALED_PACK8:
		break;
	case gli::FORMAT_BGRA8_UINT_PACK8:
		break;
	case gli::FORMAT_BGRA8_SINT_PACK8:
		break;
	case gli::FORMAT_BGRA8_SRGB_PACK8:
		break;
	case gli::FORMAT_RGBA8_UNORM_PACK32:
		break;
	case gli::FORMAT_RGBA8_SNORM_PACK32:
		break;
	case gli::FORMAT_RGBA8_USCALED_PACK32:
		break;
	case gli::FORMAT_RGBA8_SSCALED_PACK32:
		break;
	case gli::FORMAT_RGBA8_UINT_PACK32:
		break;
	case gli::FORMAT_RGBA8_SINT_PACK32:
		break;
	case gli::FORMAT_RGBA8_SRGB_PACK32:
		break;
	case gli::FORMAT_RGB10A2_UNORM_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SNORM_PACK32:
		break;
	case gli::FORMAT_RGB10A2_USCALED_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SSCALED_PACK32:
		break;
	case gli::FORMAT_RGB10A2_UINT_PACK32:
		break;
	case gli::FORMAT_RGB10A2_SINT_PACK32:
		break;
	case gli::FORMAT_BGR10A2_UNORM_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SNORM_PACK32:
		break;
	case gli::FORMAT_BGR10A2_USCALED_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SSCALED_PACK32:
		break;
	case gli::FORMAT_BGR10A2_UINT_PACK32:
		break;
	case gli::FORMAT_BGR10A2_SINT_PACK32:
		break;
	case gli::FORMAT_R16_UNORM_PACK16:
		break;
	case gli::FORMAT_R16_SNORM_PACK16:
		break;
	case gli::FORMAT_R16_USCALED_PACK16:
		break;
	case gli::FORMAT_R16_SSCALED_PACK16:
		break;
	case gli::FORMAT_R16_UINT_PACK16:
		break;
	case gli::FORMAT_R16_SINT_PACK16:
		break;
	case gli::FORMAT_RG16_UNORM_PACK16:
		break;
	case gli::FORMAT_RG16_SNORM_PACK16:
		break;
	case gli::FORMAT_RG16_USCALED_PACK16:
		break;
	case gli::FORMAT_RG16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RG16_UINT_PACK16:
		break;
	case gli::FORMAT_RG16_SINT_PACK16:
		break;
	case gli::FORMAT_RGB16_UNORM_PACK16:
		break;
	case gli::FORMAT_RGB16_SNORM_PACK16:
		break;
	case gli::FORMAT_RGB16_USCALED_PACK16:
		break;
	case gli::FORMAT_RGB16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RGB16_UINT_PACK16:
		break;
	case gli::FORMAT_RGB16_SINT_PACK16:
		break;
	case gli::FORMAT_RGB16_SFLOAT_PACK16:
		break;
	case gli::FORMAT_RGBA16_UNORM_PACK16:
		break;
	case gli::FORMAT_RGBA16_SNORM_PACK16:
		break;
	case gli::FORMAT_RGBA16_USCALED_PACK16:
		break;
	case gli::FORMAT_RGBA16_SSCALED_PACK16:
		break;
	case gli::FORMAT_RGBA16_UINT_PACK16:
		break;
	case gli::FORMAT_RGBA16_SINT_PACK16:
		break;
	case gli::FORMAT_R32_UINT_PACK32:
		break;
	case gli::FORMAT_R32_SINT_PACK32:
		break;
	case gli::FORMAT_RG32_UINT_PACK32:
		break;
	case gli::FORMAT_RG32_SINT_PACK32:
		break;
	case gli::FORMAT_RGB32_UINT_PACK32:
		break;
	case gli::FORMAT_RGB32_SINT_PACK32:
		break;
	case gli::FORMAT_RGB32_SFLOAT_PACK32:
		break;
	case gli::FORMAT_RGBA32_UINT_PACK32:
		break;
	case gli::FORMAT_RGBA32_SINT_PACK32:
		break;
	case gli::FORMAT_R64_UINT_PACK64:
		break;
	case gli::FORMAT_R64_SINT_PACK64:
		break;
	case gli::FORMAT_R64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RG64_UINT_PACK64:
		break;
	case gli::FORMAT_RG64_SINT_PACK64:
		break;
	case gli::FORMAT_RG64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RGB64_UINT_PACK64:
		break;
	case gli::FORMAT_RGB64_SINT_PACK64:
		break;
	case gli::FORMAT_RGB64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RGBA64_UINT_PACK64:
		break;
	case gli::FORMAT_RGBA64_SINT_PACK64:
		break;
	case gli::FORMAT_RGBA64_SFLOAT_PACK64:
		break;
	case gli::FORMAT_RG11B10_UFLOAT_PACK32:
		break;
	case gli::FORMAT_RGB9E5_UFLOAT_PACK32:
		break;
	case gli::FORMAT_D16_UNORM_PACK16:
		break;
	case gli::FORMAT_D24_UNORM_PACK32:
		break;
	case gli::FORMAT_D32_SFLOAT_PACK32:
		break;
	case gli::FORMAT_S8_UINT_PACK8:
		break;
	case gli::FORMAT_D16_UNORM_S8_UINT_PACK32:
		break;
	case gli::FORMAT_D24_UNORM_S8_UINT_PACK32:
		break;
	case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64:
		break;
	case gli::FORMAT_RGB_DXT1_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_DXT3_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_R_ATI1N_UNORM_BLOCK8:
		break;
	case gli::FORMAT_R_ATI1N_SNORM_BLOCK8:
		break;
	case gli::FORMAT_RG_ATI2N_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RG_ATI2N_SNORM_BLOCK16:
		break;
	case gli::FORMAT_RGB_BP_UFLOAT_BLOCK16:
		break;
	case gli::FORMAT_RGB_BP_SFLOAT_BLOCK16:
		break;
	case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_BP_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGB_ETC2_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ETC2_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ETC2_SRGB_BLOCK16:
		break;
	case gli::FORMAT_R_EAC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_R_EAC_SNORM_BLOCK8:
		break;
	case gli::FORMAT_RG_EAC_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RG_EAC_SNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_4X4_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_4X4_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X4_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X4_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_5X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_6X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_8X8_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X5_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X5_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X6_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X6_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X8_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X8_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X10_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_10X10_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X10_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X10_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X12_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ASTC_12X12_SRGB_BLOCK16:
		break;
	case gli::FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32:
		break;
	case gli::FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8:
		break;
	case gli::FORMAT_RGB_ETC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGB_ATC_UNORM_BLOCK8:
		break;
	case gli::FORMAT_RGBA_ATCA_UNORM_BLOCK16:
		break;
	case gli::FORMAT_RGBA_ATCI_UNORM_BLOCK16:
		break;
	case gli::FORMAT_L8_UNORM_PACK8:
		break;
	case gli::FORMAT_A8_UNORM_PACK8:
		break;
	case gli::FORMAT_LA8_UNORM_PACK8:
		break;
	case gli::FORMAT_L16_UNORM_PACK16:
		break;
	case gli::FORMAT_A16_UNORM_PACK16:
		break;
	case gli::FORMAT_LA16_UNORM_PACK16:
		break;
	case gli::FORMAT_BGR8_UNORM_PACK32:
		break;
	case gli::FORMAT_BGR8_SRGB_PACK32:
		break;
	case gli::FORMAT_RG3B2_UNORM_PACK8:
		break;
	default:
		break;
	}
	std::abort();
	return VK_FORMAT_UNDEFINED;
}

std::string VK_Texture::get_file_extension(CONST std::string& filename)
{
	size_t pos = filename.rfind('.');
	if (pos != std::string::npos) 
	{
		return filename.substr(pos + 1);
	}
	else 
	{
		return "";
	}
}

VK_Texture::~VK_Texture()
{
	if (texture_image != VK_NULL_HANDLE)
	{
		vkDestroyImage(device->GetDevice(), texture_image, nullptr);
		texture_image = VK_NULL_HANDLE;
	}
	if (texture_image_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device->GetDevice(), texture_image_view, nullptr);
		texture_image_view = VK_NULL_HANDLE;
	}
	if (texture_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device->GetDevice(), texture_sampler, nullptr);
		texture_sampler = VK_NULL_HANDLE;
	}
	VK_MemoryManager* memory_manager = device->GetMemoryManager();
	memory_manager->FreeAllocation(allocation);
}

void VK_Texture::GenerateImageCreateInfo(VkImageCreateInfo& image_create_info, VK_Device& in_device, CONST TextureDesc& desc)
{
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

	VkImageType image_type= VK_Utils::Translate_Texture_type_To_Vulkan(desc.type);
	VkFormat format = VK_Utils::Translate_Texture_Format_To_Vulkan(desc.format);
	VkImageCreateFlags flag = VK_Utils::Translate_Texture_type_To_VulkanCreateFlags(desc.type);
	VkSampleCountFlagBits sample_count = VK_Utils::Translate_Texture_SampleCount_To_Vulkan(desc.samples);
	VkImageUsageFlags usage = VK_Utils::Translate_Texture_usage_type_To_VulkanUsageFlags(desc.usage);
	image_create_info.imageType = image_type;
	image_create_info.extent.width = static_cast<uint32_t>(desc.width);
	image_create_info.extent.height = static_cast<uint32_t>(desc.height);
	image_create_info.extent.depth = static_cast<uint32_t>(desc.depth);
	image_create_info.mipLevels = static_cast<uint32_t>(desc.mip_level);
	image_create_info.arrayLayers = static_cast<uint32_t>(desc.layer_count);
	image_create_info.format = format;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage =
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | usage;
	image_create_info.samples = sample_count;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = flag;
	image_create_info.pNext = nullptr;
}

void VK_Texture::GenerateViewCreateInfo(VkImageViewCreateInfo& view_create_info, VK_Device& in_device, CONST TextureDesc& desc,VkImage& texture_image)
{
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.image = texture_image;
	view_create_info.viewType = VK_Utils::Translate_Texture_type_To_VulkanImageViewType(desc.type);
	view_create_info.format = VK_Utils::Translate_Texture_Format_To_Vulkan(desc.format);
	view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_create_info.subresourceRange.aspectMask = VK_Utils::Translate_Texture_type_To_VulkanImageAspectFlags(desc.type);
	view_create_info.subresourceRange.baseMipLevel = 0;
	view_create_info.subresourceRange.levelCount = static_cast<uint32_t>(desc.mip_level);
	view_create_info.subresourceRange.baseArrayLayer = 0;
	view_create_info.subresourceRange.layerCount = static_cast<uint32_t>(desc.layer_count);
	view_create_info.pNext = nullptr;
	view_create_info.flags = 0;

}
VkImage VK_Texture::GetImage() CONST
{
	return texture_image;
}
VkImageView VK_Texture::GetImageView() CONST
{
	return texture_image_view;
}
VkSampler VK_Texture::GetSampler() CONST
{
	return texture_sampler;
}

void VK_Texture::UpdateTextureData(CONST TextureDataPayload& texture_data_payload)
{
	if (texture_data_payload.data.size() == 0)
	{
		return;
	}

	VK_Buffer* staging_buffer = device->GetStagingBufferManager()->GetStagingBuffer(texture_data_payload.data.size());
	
	void* data = staging_buffer->Map();
	memcpy(data, texture_data_payload.data.data(), texture_data_payload.data.size());

	Vector<VkBufferImageCopy> buffer_copy_regions(texture_data_payload.layer_count*texture_data_payload.mip_level);
	VkDeviceSize buffer_offset = 0;
	UInt32 index = 0;
	Texture::TextureFormatAttribs fmt_attribs = Texture::GetTextureFormatAttribs(texture_data_payload.format);
	for (uint32_t layer = 0; layer < texture_data_payload.layer_count; ++layer)
	{
		for (uint32_t level = 0; level < texture_data_payload.mip_level; ++level)
		{
			VkBufferImageCopy& buffer_copy_region = buffer_copy_regions[layer*texture_data_payload.mip_level + level];
			auto mip_info = texture_data_payload.GetMipLevelProperties(level);
			buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_region.imageSubresource.mipLevel = level;
			buffer_copy_region.imageSubresource.baseArrayLayer = layer;
			buffer_copy_region.imageSubresource.layerCount = 1;
			if (texture_data_payload.type == ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP)
			{
				buffer_copy_region.imageExtent.width = static_cast<uint32_t>(texture_data_payload.width );
				buffer_copy_region.imageExtent.height = static_cast<uint32_t>(texture_data_payload.height );
			}
			else
			{
				buffer_copy_region.imageExtent.width = static_cast<uint32_t>(texture_data_payload.width >> level);
				buffer_copy_region.imageExtent.height = static_cast<uint32_t>(texture_data_payload.height >> level);
			}
			buffer_copy_region.imageExtent.depth = 1;
			buffer_copy_region.bufferOffset = buffer_offset;

			buffer_offset += mip_info.mip_size;
		}
	}
	
	VK_CommandBuffer* command_buffer = device->GetCommandBufferManager()->GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE::TRANSFER);
	command_buffer->Begin();
	VkImageSubresourceRange subres_range;
	subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subres_range.baseArrayLayer = 0;
	subres_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
	subres_range.baseMipLevel = 0;
	subres_range.levelCount = VK_REMAINING_MIP_LEVELS;
	command_buffer->TrainsitionImageLayout(texture_image, texture_image_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subres_range, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	command_buffer->MemoryBarrier(VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	texture_image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	command_buffer->CopyBufferToImage(staging_buffer->GetBuffer(), texture_image, texture_image_layout, buffer_copy_regions.size(), buffer_copy_regions.data());
	command_buffer->TransitionTextureState(this, ENUM_RESOURCE_STATE::ShaderResource);
	command_buffer->End();
	device->GetQueue(ENUM_QUEUE_TYPE::TRANSFER)->Submit(command_buffer);

	device->GetStagingBufferManager()->ReleaseStagingBuffer(staging_buffer, command_buffer);
	
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
