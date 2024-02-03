#pragma once
#ifndef _BUFFER_
#define _BUFFER_
#include "RenderEnum.h"
#include "RenderRource.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Buffer,  public RenderResource)
#pragma region METHOD
public:
	Buffer(const BufferDesc& in_buffer_desc) ;
	VIRTUAL ~Buffer() DEFAULT;

	VIRTUAL void* METHOD(Map)();
	VIRTUAL void METHOD(Unmap)();
	VIRTUAL BufferDesc METHOD(GetBufferDesc)() CONST;
protected:

private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	BufferDesc buffer_desc;
private:
#pragma endregion



MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
