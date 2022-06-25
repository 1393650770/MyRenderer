#pragma once

#ifndef _GL_TEXTURE_
#define _GL_TEXTURE_

#include"../RenderEnum.h"
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

        GL_Texture(ENUM_TEXTURE_TYPE _type, unsigned width, unsigned height, unsigned level, unsigned internalformat, unsigned dataformat);



        GL_Texture(std::string texture_path);

        static unsigned TranslateTextureTypeToGL(ENUM_TEXTURE_TYPE type);

        unsigned get_type() const;

        void unbind();

        void bind();

        unsigned get_id() const;

        void free() ;

        bool is_valid() const;

    };
    
} // namespace name

#endif