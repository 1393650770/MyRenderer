#pragma once
#ifndef _BUFFER_
#define _BUFFER_
#include "../Core/ConstDefine.h"
#include "RenderEnum.h"
#include "RenderRource.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_STRUCT(BufferDesc)
UInt32 size=0;
UInt32 stride=0;
ENUM_BUFFER_TYPE usage = ENUM_BUFFER_TYPE::None;

BufferDesc(const BufferDesc& other)
{
	*this=other;
}

BufferDesc& operator=(const BufferDesc& other)
{
	size = other.size;
	stride = other.stride;
	usage = other.usage;
	return *this;
}

MYRENDERER_END_STRUCT

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
	BufferDesc buffer_desc;
protected:

private:
#pragma endregion



MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
