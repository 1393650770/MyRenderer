#include"MyTexture.h"
#include<iostream>
#include <glad/glad.h>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
TextureDesc::TextureDesc(const TextureDesc& other)
{
    *this= other;
}

Texture::Texture(const TextureDesc in_texture_desc):texture_desc(in_texture_desc)
{
    
}

TextureDesc Texture::GetTextureDesc() const
{
    return texture_desc;
}


MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
