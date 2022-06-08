#pragma once

#ifndef _GL_TEXTURE_
#define _GL_TEXTURE_

#include"GL_ENUM.h"

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
        GL_Texture();
        GL_Texture(unsigned _id , ENUM_TEXTURE_TYPE _type);
        virtual ~GL_Texture();
    };
    
} // namespace name

#endif