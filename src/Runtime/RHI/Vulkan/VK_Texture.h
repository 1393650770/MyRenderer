#pragma once

#ifndef _VK_TEXTURE_
#define _VK_TEXTURE_

#include"../RenderEnum.h"

#include <vector>
#include <string>
#include "vulkan/vulkan_core.h"

namespace MXRender
{

    class VK_Texture
    {
    private:
        ENUM_TEXTURE_TYPE type;
		unsigned id;
        VkAttachmentDescription attachment_description;
        VkAttachmentReference attachment_reference;
        unsigned width,height, texChannels;
        VkDeviceSize imageSize;
    public:
        virtual ~VK_Texture();
        VK_Texture();
        VK_Texture(unsigned _id , ENUM_TEXTURE_TYPE _type);
        VK_Texture(std::vector<std::string>& cubemap_texture);

        VK_Texture(ENUM_TEXTURE_TYPE _type, unsigned width, unsigned height, unsigned level, unsigned internalformat, unsigned dataformat, ENUM_TEXTURE_USAGE_TYPE usage_type= ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_NOT_VALID);



        VK_Texture(std::string texture_path);

        unsigned get_type() const;

        void unbind();

        void bind();

        unsigned get_id() const;

        void free() ;

        bool is_valid() const;

        VkAttachmentDescription& get_attachment_description();
        VkAttachmentReference& get_attachment_reference();
    };
    
} // namespace name

#endif