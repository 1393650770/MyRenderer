#pragma once

#ifndef _VK_TEXTURE_
#define _VK_TEXTURE_

#include"../RenderEnum.h"

#include <vector>
#include <string>
#include "vulkan/vulkan_core.h"
#include "gli/format.hpp"

namespace MXRender
{

    class VK_Texture
    {
    private:
        ENUM_TEXTURE_TYPE type=ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID;
		unsigned id;
        VkAttachmentDescription attachment_description;
        VkAttachmentReference attachment_reference;
        unsigned width,height, texChannels;
        VkDeviceSize imageSize;
        void load_dds(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
        void load_dds_cubemap(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
        void load_dds_2d(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
        void load_common_2d(const std::string& texture_path);
        VkFormat trans_gli_format_to_vulkan(gli::format format);
		std::string get_file_extension(const std::string& filename) ;

    public:
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;

        virtual ~VK_Texture();
        VK_Texture();
        VK_Texture(unsigned _id , ENUM_TEXTURE_TYPE _type);
        VK_Texture(std::vector<std::string>& cubemap_texture);
        VK_Texture(ENUM_TEXTURE_TYPE _type, const std::string& texture_path);
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