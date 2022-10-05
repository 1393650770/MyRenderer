#pragma once
#ifndef _GL_FRAMEBUFFER_
#define _GL_FRAMEBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"GL_Texture.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include"../FrameBuffer.h"

namespace MXRender
{
   class GL_Framebuffer : public Framebuffer
   {
   private:
       unsigned id;
       bool b_is_valid=false;

	   std::vector<GL_Texture> color_textures;
       GL_Texture depth_texture;
       void init_framebuffer();
   public:
       GL_Framebuffer(/* args */);
       GL_Framebuffer(unsigned width,unsigned height,const FrameBufferAttachmentDescriptor& attachment);
       void bind() override;
       void bind_read_framebuffer() override;
       void bind_draw_framebuffer() override;
       void unbind() override;

       unsigned get_color_attachment_id(unsigned index = 0) const override;
       unsigned get_depth_attachment_id() const override;

       void resize(unsigned new_width, unsigned new_height) override;

       unsigned get_id() override;

       void free() override ;

       virtual ~GL_Framebuffer();
   };
   

   
}
#endif
