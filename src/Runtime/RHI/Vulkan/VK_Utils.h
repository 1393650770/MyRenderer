#pragma once
#ifndef _VK_UTILS_
#define _VK_UTILS_

#include<vulkan/vulkan.h>
#include"glm/glm.hpp"
#include"VK_Device.h"
#include"RHI/RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <functional>

#include "vma/vk_mem_alloc.h"

namespace MXRender { class VK_GraphicsContext; }


namespace MXRender { struct MipmapInfo; }
namespace MXRender
{

	class AllocatedImage {
	public:
		VkImage _image;
		VmaAllocation _allocation;
		VkImageView _defaultView;
		int mipLevels;
	};

	class AllocatedBufferUntyped {
	public:
		VkBuffer _buffer{VK_NULL_HANDLE};
		VmaAllocation _allocation{};
		VkDeviceSize _size{ 0 };
		VkDescriptorBufferInfo get_info(VkDeviceSize offset = 0);
	};

	template<typename T>
	class AllocatedBuffer : public AllocatedBufferUntyped {
	public:
		void operator=(const AllocatedBufferUntyped& other) {
			_buffer = other._buffer;
			_allocation = other._allocation;
			_size = other._size;
		}
		AllocatedBuffer(AllocatedBufferUntyped& other) {
			_buffer = other._buffer;
			_allocation = other._allocation;
			_size = other._size;
		}
		AllocatedBuffer() = default;
	};
	

    class VK_Utils
    {
    private:
       
    public:
		static VkAccessFlags METHOD(AccessMaskFromImageLayout)(VkImageLayout Layout, bool IsDstMask);

		static void Destroy_Buffer(VK_GraphicsContext* context, AllocatedBufferUntyped& buffer);
		static void* Map_Buffer(VK_GraphicsContext* context, AllocatedBufferUntyped& buffer);
		static void Unmap_Buffer(VK_GraphicsContext* context, AllocatedBufferUntyped& buffer);
		static VkImageViewCreateInfo Imageview_Create_Info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
		static VkImageCreateInfo Image_Create_Info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
		static void Immediate_Submit(VK_GraphicsContext* context, std::function<void(VkCommandBuffer cmd)>&& function);
		static AllocatedImage Upload_Image_Mipmapped(VK_GraphicsContext* context, int texWidth, int texHeight, VkFormat image_format, AllocatedBufferUntyped& stagingBuffer, std::vector<MipmapInfo> mips);
		static bool Load_Image_From_Asset(VK_GraphicsContext* context, const char* filename, AllocatedImage& outImage);
		static VkSamplerCreateInfo Sampler_Create_Info(VkFilter filters, VkSamplerAddressMode samplerAdressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		static VkPipelineDepthStencilStateCreateInfo Depth_Stencil_Create_Info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
		static VkPipelineColorBlendAttachmentState Color_Blend_Attachment_State();
		static VkPipelineMultisampleStateCreateInfo Multisampling_State_Create_Info();
		static VkPipelineInputAssemblyStateCreateInfo Input_Assembly_Create_Info(VkPrimitiveTopology topology);
		static VkPipelineRasterizationStateCreateInfo Rasterization_State_Create_Info(VkPolygonMode polygonMode);
		static VkPipelineShaderStageCreateInfo Pipeline_Shader_Stage_Create_Info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
		static VkPipelineVertexInputStateCreateInfo Vertex_Input_State_Create_Info();
		static VkPipelineLayoutCreateInfo Pipeline_Layout_Create_Info();
		static uint32_t Hash_Descriptor_Layout_Info(VkDescriptorSetLayoutCreateInfo* info);
		//static void Create_VKBuffer(std::weak_ptr< VK_Device> Device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory);
		static AllocatedBufferUntyped Create_buffer(VK_GraphicsContext* context, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags required_flags);
		static void Copy_VKBuffer(std::weak_ptr< VK_GraphicsContext> context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		static void Copy_VKBuffer(VK_GraphicsContext* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		static void           Create_Image(VkPhysicalDevice      physical_device,
			VkDevice              device,
			uint32_t              image_width,
			uint32_t              image_height,
			VkFormat              format,
			VkImageTiling         image_tiling,
			VkImageUsageFlags     image_usage_flags,
			VkMemoryPropertyFlags memory_property_flags,
			VkImage& image,
			VkDeviceMemory& memory,
			VkImageCreateFlags    image_create_flags,
			uint32_t              array_layers,
			uint32_t              miplevels);
		static VkImageView    Create_ImageView(VkDevice           device,
			VkImage& image,
			VkFormat           format,
			VkImageAspectFlags image_aspect_flags,
			VkImageViewType    view_type,
			uint32_t           layout_count,
			uint32_t           miplevels);

		//static uint32_t Find_MemoryType(std::weak_ptr< VK_Device> Device, uint32_t TypeFilter, VkMemoryPropertyFlags Properties);
		static VkSampleCountFlagBits Get_SampleCountFlagBits_FromInt(unsigned num);
		static VkBufferUsageFlags Translate_Buffer_usage_type_To_VulkanUsageFlag(const ENUM_BUFFER_TYPE& usage_type);
		static VkImageLayout Translate_Texture_usage_type_To_Vulkan(const ENUM_TEXTURE_USAGE_TYPE& usage_type);
		static VkImageUsageFlags Translate_Texture_usage_type_To_VulkanUsageFlags(const ENUM_TEXTURE_USAGE_TYPE& usage_type);
		static VkImageType Translate_Texture_type_To_Vulkan(const ENUM_TEXTURE_TYPE& type);
		static VkFormat Translate_Texture_Format_To_Vulkan(const ENUM_TEXTURE_FORMAT& format);
		static VkImageViewType Translate_Texture_type_To_VulkanImageViewType(const ENUM_TEXTURE_TYPE& type);
		static VkImageAspectFlags Translate_Texture_type_To_VulkanImageAspectFlags(const ENUM_TEXTURE_TYPE& type);
		static VkImageCreateFlags Translate_Texture_type_To_VulkanCreateFlags(const ENUM_TEXTURE_TYPE& type);
		static VkSampleCountFlagBits Translate_Texture_SampleCount_To_Vulkan(const UInt8& sample_count);
		static VkFormat Translate_API_DataTypeEnum_To_Vulkan(ENUM_RENDER_DATA_TYPE data_type);
		static VkShaderStageFlagBits Translate_API_ShaderTypeEnum_To_Vulkan(ENUM_SHADER_STAGE shader_type);
		static void ClearImageColor(std::weak_ptr< VK_GraphicsContext> context,
			VkImageLayout      image_layout,
			VkImage            image,
			VkImageAspectFlags imageAspectflags);
		static void Transition_ImageLayout(std::weak_ptr< VK_GraphicsContext> context,
			VkImage            image,
			VkImageLayout      old_layout,
			VkImageLayout      new_layout,
			uint32_t           layer_count,
			uint32_t           miplevels,
			VkImageAspectFlags aspect_mask_bits);
		static void Transition_ImageLayout(std::weak_ptr< VK_GraphicsContext> context,
			VkCommandBuffer commandbuffer,
			VkImage            image,
			VkImageLayout      old_layout,
			VkImageLayout      new_layout,
			uint32_t           layer_count,
			uint32_t           miplevels,
			VkImageAspectFlags aspect_mask_bits);

		static void	Copy_Buffer_To_Image(std::weak_ptr< VK_GraphicsContext> context,
			VkBuffer buffer,
			VkImage  image,
			uint32_t width,
			uint32_t height,
			uint32_t layer_count);
		static void	Copy_Buffer_To_Image(std::weak_ptr< VK_GraphicsContext> context,
			VkBuffer buffer,
			VkImage  image,
			uint32_t width,
			uint32_t height,
			uint32_t layer_count,
			uint32_t level,
			uint32_t baseArrayLayer,
			uint32_t bufferOffset);

		static VkSampler Create_Linear_Sampler(VkPhysicalDevice physical_device, VkDevice device);
    };
    
}

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

template<typename BitsType>
constexpr bool VKHasAllFlags(VkFlags flags, BitsType contains)
{
	return (flags & contains) == contains;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
