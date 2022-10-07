#include"VK_Texture.h"
#include<iostream>
#include <glad/glad.h>
#include"../../../ThirdParty/stb_image/stb_image.h"
#include "VK_Utils.h"
namespace MXRender
{
    VK_Texture::VK_Texture(/* args */)
    {
    }

    VK_Texture::VK_Texture(unsigned _id, ENUM_TEXTURE_TYPE _type)
    {
        id = _id;
        type = _type;
    }

    VK_Texture::VK_Texture(std::vector<std::string>& cubemap_texture)
    {
        if (is_valid())
            return;
       
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

    }
 
    
} // namespace MX
