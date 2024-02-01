#include "FrameBuffer.h"

#include"RenderState.h"
namespace MXRender
{
    std::shared_ptr<Framebuffer> Framebuffer::CreateFramebuffer(unsigned width, unsigned height, const FrameBufferAttachmentDescriptor& attachment)
    {
        switch (RenderState::render_api_type)
        {
        case ENUM_RENDER_API_TYPE::OpenGL: 
            return nullptr;
            break;
        default:
            return nullptr;
            break;
        }
        return nullptr;
    }

}