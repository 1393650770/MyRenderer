#pragma once
#ifndef _FRAMEBUFFER_
#define _FRAMEBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderEnum.h"


namespace MXRender
{
    class Framebuffer
    {
    private:
		//std::vector<Texture> color_textures;
  //      Texture depth_texture;
    public:
        //÷°ª∫≥ÂTexture√Ë ˆ∑˚
        struct FrameBufferTextureDescriptor
        {
        public:
            FrameBufferTextureDescriptor() = default;
            FrameBufferTextureDescriptor(ENUM_TEXTURE_FORMAT attachment)
                : attachment_format(attachment) {}
            ENUM_TEXTURE_FORMAT attachment_format;
            //∂‡÷ÿ≤…—˘ ˝
            unsigned samples = 1;
        };

        //÷°ª∫≥ÂAttachment√Ë ˆ∑˚
        struct FrameBufferAttachmentDescriptor
        {
        public:
            FrameBufferAttachmentDescriptor() = default;
            FrameBufferAttachmentDescriptor(std::initializer_list<FrameBufferTextureDescriptor> attachments)
                : attachment_format(attachments) {}
            std::vector<FrameBufferTextureDescriptor> attachment_format;
        };

        unsigned width, height;
        FrameBufferAttachmentDescriptor attachment_format_list;
        
        virtual void bind() = 0;
        virtual void bind_read_framebuffer() = 0;
        virtual void bind_draw_framebuffer() = 0;
        virtual void unbind() = 0;
       
        virtual unsigned get_color_attachment_id(unsigned index = 0) const = 0;
        virtual unsigned get_depth_attachment_id() const = 0;
        

        virtual void resize(unsigned new_width, unsigned new_height) = 0;

        virtual unsigned get_id()=0;

        virtual void free() = 0;

        virtual ~Framebuffer() = default;
        
        static std::shared_ptr<Framebuffer> CreateFramebuffer(unsigned width, unsigned height, const FrameBufferAttachmentDescriptor& attachment);
    };



}
#endif
