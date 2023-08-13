#pragma once

#ifndef _VK_TEXTURE_
#define _VK_TEXTURE_

#include"../RenderEnum.h"

#include <vector>
#include <string>
#include "vulkan/vulkan_core.h"
#include "gli/format.hpp"
#include "../../Core/ConstDefine.h"



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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Texture,public RenderResource)
    
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
        VkImageLayout textureImageLayout;
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
MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif