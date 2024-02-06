#pragma once
#ifndef _VK_FRAMEBUFFER_
#define _VK_FRAMEBUFFER_
#include "RHI/RenderFrameBuffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_FrameBuffer,public FrameBuffer)

#pragma region METHOD
    public:
        VK_FrameBuffer(VK_Device* in_device, CONST FrameBufferDesc& in_desc);
        VIRTUAL ~VK_FrameBuffer() ;
    protected:

    private:
#pragma endregion

#pragma region MEMBER
	public:

	protected:
        VK_Device* device;  
	private:
#pragma endregion
        
MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE


#endif
