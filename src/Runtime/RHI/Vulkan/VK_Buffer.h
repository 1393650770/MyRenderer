#pragma once
#ifndef _VK_BUFFER_
#define _VK_BUFFER_



#include "../Buffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Buffer,  public Buffer)
#pragma region METHOD
public:
VK_Buffer(const BufferDesc in_buffer_desc) ;
VIRTUAL ~VK_Buffer() DEFAULT;

VIRTUAL void* METHOD(Map)() OVERRIDE;
VIRTUAL void METHOD(Unmap)() OVERRIDE;

protected:

private:

#pragma endregion


#pragma region MEMBER
public:

protected:

private:
#pragma endregion



MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
