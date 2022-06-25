#include "GL_Framebuffer.h"
#include"GLFW/glfw3.h"
namespace MXRender
{
    void GL_Framebuffer::init_framebuffer()
    {
        if (b_is_valid)
        {
            glDeleteFramebuffers(1, &id);
            depth_texture.free();
            color_textures.clear();
        }

        glGenFramebuffers(1, &id);
        bind();
        for (unsigned i = 0; i < attachment_format_list.attachment_format.size(); i++)
        {
            ENUM_TEXTURE_TYPE type = ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID;
            if (attachment_format_list.attachment_format[i].samples <= 1)
            {
                type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
            }
            else
            {
                type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_MULTISAMPLE;
            }
            switch (attachment_format_list.attachment_format[i].attachment_format)
            {
            case ENUM_TEXTURE_FORMAT::RGBA16:
            case ENUM_TEXTURE_FORMAT::RGBA16F:
                color_textures.emplace_back(type, width, height, attachment_format_list.attachment_format[i].samples, GL_RGBA16F, GL_UNSIGNED_BYTE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_textures[color_textures.size() - 1].get_type(), color_textures[color_textures.size() - 1].get_id(), 0);
                break;
            case ENUM_TEXTURE_FORMAT::RGBA8:
                color_textures.emplace_back(type, width, height, attachment_format_list.attachment_format[i].samples, GL_RGBA8, GL_UNSIGNED_BYTE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_textures[color_textures.size() - 1].get_type(), color_textures[color_textures.size() - 1].get_id(), 0);
                break;
            case ENUM_TEXTURE_FORMAT::D0S8:
            case ENUM_TEXTURE_FORMAT::D16:
            case ENUM_TEXTURE_FORMAT::D16F:
            case ENUM_TEXTURE_FORMAT::D32:
            case ENUM_TEXTURE_FORMAT::D32F:
                depth_texture = GL_Texture(ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH, width, height, attachment_format_list.attachment_format[i].samples, GL_RGBA8, GL_UNSIGNED_BYTE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture.get_type(), depth_texture.get_id(), 0);
                break;
            default:

                break;
            }

        }
        unbind();
    }
    GL_Framebuffer::GL_Framebuffer(/* args */)
    {
    }
   
    GL_Framebuffer::~GL_Framebuffer()
    {
        free();
    }

    GL_Framebuffer::GL_Framebuffer(unsigned width, unsigned height, const FrameBufferAttachmentDescriptor& attachment)
    {
        if (b_is_valid)
            return;

        this->width = width;
        this->height = height;
        attachment_format_list = attachment;

        init_framebuffer();

        b_is_valid = true;
    }


    void GL_Framebuffer::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glViewport(0,0,width,height);
    }
    void GL_Framebuffer::bind_read_framebuffer()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
    }
    void GL_Framebuffer::bind_draw_framebuffer()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
    }
    void GL_Framebuffer::unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned GL_Framebuffer::get_color_attachment_id(unsigned index) const
    {
        if(index<0|| index>=color_textures.size())
            return -1;
        return color_textures[index].get_id();
    }
    unsigned GL_Framebuffer::get_depth_attachment_id() const
    {
        return depth_texture.get_id();
    }


    void GL_Framebuffer::resize(unsigned new_width, unsigned new_height)
    {
        if (new_width == 0 || new_height == 0)
            return;

        width = new_width;
        height = new_height;

        init_framebuffer();
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