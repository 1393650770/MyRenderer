#include"GL_Texture.h"
#include<iostream>
#include <glad/glad.h>
#include"../../../ThirdParty/stb_image/stb_image.h"
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

    GL_Texture::GL_Texture(std::vector<std::string>& cubemap_texture)
    {
        if (is_valid())
            return;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        int width, height, nrComponents;
        for (unsigned int i = 0; i < cubemap_texture.size(); i++)
        {
            unsigned char* data = stbi_load(cubemap_texture[i].c_str(), &width, &height, &nrComponents, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << cubemap_texture[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        type = ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
        unbind();

    }

    GL_Texture::GL_Texture(ENUM_TEXTURE_TYPE _type, unsigned width,unsigned height, unsigned samples, unsigned internalformat, unsigned dataformat)
    {
        if (is_valid())
            return;
        glGenTextures(1, &id);

        switch (_type)
        {
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID:
            break;
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D:
        {    
            type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
            bind();

            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, dataformat, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            break;
        }
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE:
        {    
            type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE;
            bind();

            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat, width, height, GL_TRUE);
            break;
        }
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH:
        {
            type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH;
            bind();

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            GLfloat borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            break;
        }
        default:

            break;
        }

        unbind();
    }

    GL_Texture::GL_Texture(std::string texture_path)
    {
        if (is_valid())
            return;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        int texture_width, texture_height, nrComponents;
        unsigned char* data = stbi_load(texture_path.c_str(), &texture_width, &texture_height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture2D failed to load at path: " << texture_path << std::endl;
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
        unbind();
    }

    unsigned GL_Texture::TranslateTextureTypeToGL(ENUM_TEXTURE_TYPE type)
    {
        unsigned gl_type=0;
        switch (type)
        {
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID:
            break;
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D:
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DYNAMIC:
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH:
            gl_type = GL_TEXTURE_2D;
            break;
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE:
            gl_type = GL_TEXTURE_2D_MULTISAMPLE;
            break;
        case MXRender::ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP:
            gl_type = GL_TEXTURE_CUBE_MAP;
            break;
        default:
            break;
        }
        return gl_type;
    }

    unsigned GL_Texture::get_type() const
    {
        return TranslateTextureTypeToGL(type);
    }
    
    void GL_Texture::unbind()
    {
        if (!is_valid())
            return;

        glBindTexture(TranslateTextureTypeToGL(type), 0);
    }

    void GL_Texture::bind()
    {
        if (!is_valid())
            return;

        glBindTexture(TranslateTextureTypeToGL(type), id);
    }

    unsigned GL_Texture::get_id() const
    {
        if (!is_valid())
            return 0;
        return id;
    }

    void GL_Texture::free()
    {
        if (!is_valid())
            return;

        glDeleteTextures(1, &id);
        id = -1;
        type = ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID;
    }

    bool GL_Texture::is_valid() const
    {
        return type != ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID;
    }

    GL_Texture::~GL_Texture()
    {
        free();
    }
 
    
} // namespace MX
