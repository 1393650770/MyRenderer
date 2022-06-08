#include"GL_Texture.h"

namespace MXRender
{
    GL_Texture::GL_Texture(/* args */)
    {
    }

    GL_Texture::GL_Texture(unsigned _id, ENUM_TEXTURE_TYPE _type)
    {
        id = _id;
        type = _type;
    }
    
    GL_Texture::~GL_Texture()
    {
    }
 
    
} // namespace MX
