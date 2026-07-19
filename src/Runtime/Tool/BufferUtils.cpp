#include "BufferUtils.h"
#include <cstring>
#include "RHI/RenderRHI.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderBuffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

RHI::Buffer* BufferUtils::CreateStorageBuffer(UInt32 in_size_bytes, UInt32 in_stride, ENUM_BUFFER_TYPE in_extra_flags)
{
	RHI::BufferDesc desc;
	desc.size = in_size_bytes;
	desc.stride = in_stride;
	desc.type = ENUM_BUFFER_TYPE::Storage | in_extra_flags;
	return RHICreateBuffer(desc);
}

RHI::Buffer* BufferUtils::CreateDynamicParamBuffer(UInt32 in_size_bytes)
{
	RHI::BufferDesc desc;
	desc.size = in_size_bytes;
	desc.stride = in_size_bytes;
	desc.type = ENUM_BUFFER_TYPE::Storage | ENUM_BUFFER_TYPE::Dynamic;
	return RHICreateBuffer(desc);
}

void BufferUtils::Upload(RHI::Buffer* in_buffer, CONST void* in_data, UInt32 in_size_bytes, UInt32 in_dst_offset)
{
	void* mapped = RHIMapBuffer(in_buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy((UInt8*)mapped + in_dst_offset, in_data, in_size_bytes);
	RHIUnmapBuffer(in_buffer);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
