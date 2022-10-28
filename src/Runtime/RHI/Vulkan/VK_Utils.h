#pragma once
#ifndef _VK_UTILS_
#define _VK_UTILS_

#include<vulkan/vulkan.h>
#include"glm/glm.hpp"
#include"VK_Device.h"
#include"../RenderEnum.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
    class VK_Utils
    {
    private:
       
    public:
        static void Create_VKBuffer(std::weak_ptr< VK_Device> Device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory);
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
        
        static uint32_t Find_MemoryType(std::weak_ptr< VK_Device> Device, uint32_t TypeFilter, VkMemoryPropertyFlags Properties);
        static VkSampleCountFlagBits Get_SampleCountFlagBits_FromInt(unsigned num);
        static VkImageLayout Translate_Texture_usage_type_To_Vulkan(const ENUM_TEXTURE_USAGE_TYPE& usage_type );
        static VkImageType Translate_Texture_type_To_Vulkan(const ENUM_TEXTURE_TYPE& type);
        static VkFormat Translate_API_DataTypeEnum_To_Vulkan(ENUM_RENDER_DATA_TYPE data_type);

		static void Transition_ImageLayout(std::weak_ptr< VK_GraphicsContext> context,
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
			uint32_t layer_coun);


		static VkSampler Create_Linear_Sampler(VkPhysicalDevice physical_device, VkDevice device);
    };
    
}
#endif
