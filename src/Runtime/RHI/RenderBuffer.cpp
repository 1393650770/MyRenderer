#include "RenderBuffer.h"
#include "RenderUtils.h"
#include "RenderRHI.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

Buffer::Buffer(const BufferDesc& in_buffer_desc) :buffer_desc(in_buffer_desc)
{
}

BufferDesc Buffer::GetBufferDesc() const
{
	return buffer_desc;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE

