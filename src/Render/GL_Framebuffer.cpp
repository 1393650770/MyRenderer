#include "GL_Framebuffer.h"
#include"GLFW/glfw3.h"
namespace MXRender
{
    GL_Framebuffer::GL_Framebuffer(/* args */)
    {
    }
   
    GL_Framebuffer::~GL_Framebuffer()
    {
        free();
    }

    GL_Framebuffer::GL_Framebuffer(std::vector<unsigned> texture_buffer)
    {
        if (b_is_valid)
            return;
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        for (unsigned i = 0; i < texture_buffer.size(); i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture_buffer[i], 0);
            color_textures.push_back(GL_Texture(texture_buffer[i], ENUM_TEXTURE_TYPE::ENUM_TYPE_2D));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        b_is_valid = true;
    }
    unsigned GL_Framebuffer::get_id()
    {
        return id;
    }
    void GL_Framebuffer::free()
    {
        if (!b_is_valid)
            return;
        glDeleteFramebuffers(1, &id);
        b_is_valid = false;
    }
}