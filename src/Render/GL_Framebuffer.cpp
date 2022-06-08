#include "GL_Framebuffer.h"
#include"GLFW/glfw3.h"
namespace MXRender
{
    GL_Framebuffer::GL_Framebuffer(/* args */)
    {
    }
   
    GL_Framebuffer::~GL_Framebuffer()
    {
    }

    GL_Framebuffer::GL_Framebuffer( unsigned width, unsigned height)
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        //Mrt ×î´óÎª4
        for (unsigned i = 0; i < 4; i++)
        {
            unsigned colorBufferID;
            glGenTextures(1, &colorBufferID);
            glBindTexture(GL_TEXTURE_2D, colorBufferID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferID, 0);
            color_textures.push_back(GL_Texture(colorBufferID, ENUM_TEXTURE_TYPE::ENUM_TYPE_2D));
        }

    }   
}