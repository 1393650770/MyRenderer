#include"VK_Texture.h"
#include<iostream>
#include <glad/glad.h>
#include"../../../ThirdParty/stb_image/stb_image.h"
#include "VK_Utils.h"
#include "../RenderRource.h"
#include "../RenderUtils.h"
#include "../../Render/DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "VK_GraphicsContext.h"
namespace MXRender
{
    VK_Texture::VK_Texture(/* args */)
    {
    }

    VK_Texture::VK_Texture(unsigned _id, ENUM_TEXTURE_TYPE _type)
    {
		if (is_valid())
			return;
        id = _id;
        type = _type;
    }

    VK_Texture::VK_Texture(std::vector<std::string>& cubemap_texture)
    {
        if (is_valid())
            return;

        std::weak_ptr<VK_Device> device=Singleton<DefaultSetting>::get_instance().context->device;
        if(device.expired()) return ;
        
		int miplevels=1;
		type= ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
        std::vector<std::shared_ptr<TextureData>> cubemap_testure_data(6);
        for (int i=0;i<cubemap_texture.size();i++)
        {
            cubemap_testure_data[i]=RenderUtils::Load_Texture(cubemap_texture[i],true);
        }
		VkDeviceSize texture_layer_byte_size;
		VkDeviceSize cube_byte_size;
		VkFormat     vulkan_image_format;

        switch (cubemap_testure_data[0]->format)
        {       
		case ENUM_TEXTURE_FORMAT::RGBA8S:
		{
			texture_layer_byte_size = cubemap_testure_data[0]->width * cubemap_testure_data[0]->height * 4;
			vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		}
		case ENUM_TEXTURE_FORMAT::RGBA8U:
		{
			texture_layer_byte_size = cubemap_testure_data[0]->width * cubemap_testure_data[0]->height * 4;
			vulkan_image_format = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		}
		default:
		{
			texture_layer_byte_size = VkDeviceSize(-1);
			throw std::runtime_error("invalid texture_layer_byte_size");
			break;
		}
        }
        cube_byte_size = texture_layer_byte_size * 6;
		uint32_t texture_image_width= cubemap_testure_data[0]->width;
	    uint32_t texture_image_height= cubemap_testure_data[0]->height;
		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = static_cast<uint32_t>(texture_image_width);
		image_create_info.extent.height = static_cast<uint32_t>(texture_image_height);
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = miplevels;
		image_create_info.arrayLayers = 6;
		image_create_info.format = vulkan_image_format;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage =
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device.lock()->device, &image_create_info, nullptr, &textureImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.lock()->device, textureImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VK_Utils::Find_MemoryType(device,memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device.lock()->device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device.lock()->device, textureImage, textureImageMemory, 0);

		VkBuffer       inefficient_staging_buffer;
		VkDeviceMemory inefficient_staging_buffer_memory;
		VK_Utils::Create_VKBuffer(device, 
            cube_byte_size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			inefficient_staging_buffer,
			inefficient_staging_buffer_memory);

		void* data;
		vkMapMemory(device.lock()->device, inefficient_staging_buffer_memory, 0, cube_byte_size, 0, &data);
		for (int i = 0; i < 6; i++)
		{
			memcpy((void*)(static_cast<char*>(data) + texture_layer_byte_size * i),
				cubemap_testure_data[i]->pixels,
				static_cast<size_t>(texture_layer_byte_size));
		}
		vkUnmapMemory(device.lock()->device, inefficient_staging_buffer_memory);
		VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			6,
			miplevels,
			VK_IMAGE_ASPECT_COLOR_BIT);
		VK_Utils::Copy_Buffer_To_Image(Singleton<DefaultSetting>::get_instance().context, inefficient_staging_buffer,
			textureImage,
			static_cast<uint32_t>(texture_image_width),
			static_cast<uint32_t>(texture_image_height),
			6);
		VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			miplevels,
			VK_IMAGE_ASPECT_COLOR_BIT);
		vkDestroyBuffer(device.lock()->device, inefficient_staging_buffer, nullptr);
		vkFreeMemory(device.lock()->device, inefficient_staging_buffer_memory, nullptr);

		textureImageView = VK_Utils::Create_ImageView(device.lock()->device, 
			textureImage,
			vulkan_image_format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_CUBE,
			6,
			miplevels);

		textureSampler=VK_Utils::Create_Linear_Sampler(device.lock()->gpu,device.lock()->device);
    }

    VK_Texture::VK_Texture(ENUM_TEXTURE_TYPE _type, unsigned width,unsigned height, unsigned samples, unsigned internalformat, unsigned dataformat, ENUM_TEXTURE_USAGE_TYPE usage_type)
    {
        if (is_valid())
            return;

        attachment_description.samples= VK_Utils::Get_SampleCountFlagBits_FromInt(samples);
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_description.finalLayout = VK_Utils::Translate_Texture_usage_type_To_Vulkan(usage_type);

        //TODO:指定format
        attachment_description.format = VK_FORMAT_UNDEFINED;

        //TODO:设定attachment_reference
		attachment_reference.attachment = 0;
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		
    }

    VK_Texture::VK_Texture(std::string texture_path)
    {

    }


    unsigned VK_Texture::get_type() const
    {   
        return 0;
    }
    
    void VK_Texture::unbind()
    {

    }

    void VK_Texture::bind()
    {

    }

    unsigned VK_Texture::get_id() const
    {
        if (!is_valid())
            return 0;
        return id;
    }

    void VK_Texture::free()
    {
        if (!is_valid())
            return;

    }

    bool VK_Texture::is_valid() const
    {
        return type != ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID;
    }

	VkAttachmentDescription& VK_Texture::get_attachment_description()
	{
        return attachment_description;
	}

	VkAttachmentReference& VK_Texture::get_attachment_reference()
	{
        return attachment_reference;
	}


	

	VK_Texture::~VK_Texture()
    {
		std::weak_ptr<VK_Device> device = Singleton<DefaultSetting>::get_instance().context->device;
		if (device.expired()) return;
		vkDestroySampler(device.lock()->device, textureSampler, nullptr);
		vkDestroyImageView(device.lock()->device, textureImageView, nullptr);

		vkDestroyImage(device.lock()->device, textureImage, nullptr);
		vkFreeMemory(device.lock()->device, textureImageMemory, nullptr);
    }
 
    
} // namespace MX
