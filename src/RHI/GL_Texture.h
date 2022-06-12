#pragma once

#ifndef _GL_TEXTURE_
#define _GL_TEXTURE_

#include"GL_ENUM.h"
#include"GLFW/glfw3.h"
#include <vector>
#include <string>

namespace MXRender
{

    class GL_Texture
    {
    private:
        ENUM_TEXTURE_TYPE type;
		unsigned id;
    public:
        virtual ~GL_Texture();
        GL_Texture();
        GL_Texture(unsigned _id , ENUM_TEXTURE_TYPE _type);
        GL_Texture(std::vector<std::string>& cubemap_texture);

        GL_Texture(unsigned width, unsigned height, unsigned internalformat, unsigned dataformat);

        GL_Texture(std::string texture_path);

        static unsigned TranslateTextureTypeToGL(ENUM_TEXTURE_TYPE type);

        void unbind();

        void bind();

        unsigned get_id();

        void free();

        bool is_valid();

    };
    
} // namespace name

#endif