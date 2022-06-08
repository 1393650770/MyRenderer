#pragma once
#ifndef _GL_FRAMEBUFFER_
#define _GL_FRAMEBUFFER_

#include <glad/glad.h>
#include <glm/glm.hpp>
#include"GL_ENUM.h"
#include"GL_Texture.h"
#include <string>
#include<vector>
#include <fstream>
#include <sstream>
#include <iostream>
namespace MXRender
{
   class GL_Framebuffer
   {
   private:
       ENUM_FRAMEBUFFER_TYPE type;
       unsigned id;
       unsigned width;
       unsigned height;
       bool b_is_valid;
       std::vector<GL_Texture> color_textures;
       GL_Texture depth_texture;
   public:
       GL_Framebuffer(/* args */);
       GL_Framebuffer( unsigned width,unsigned height);
       
       virtual ~GL_Framebuffer();
   };
   

   
}
#endif
