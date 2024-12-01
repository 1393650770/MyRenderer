#include "RenderUtils.h"
#include "RenderBuffer.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

StreamingBuffer::StreamingBuffer(ENUM_BUFFER_TYPE in_buffer_type, UInt32 in_size, UInt32 in_num_contexts, CONST String in_name)
	:name(in_name)
{
	BufferDesc desc;
	desc.size = in_size;
	desc.type = in_buffer_type;
	desc.stride = in_size;
	buffer = RHICreateBuffer(desc);
}

StreamingBuffer::~StreamingBuffer()
{
	delete buffer;
	buffer = nullptr;
}

void StreamingBuffer::AllowPersistentMapping(Bool AllowMapping)
{
	allow_persistent_map = AllowMapping;
}

void* StreamingBuffer::GetMappedCPUAddress(UInt32 ctx_num)
{
	return map_infos[ctx_num].mapped_data;
}

Buffer* StreamingBuffer::GetBuffer()
{
	return buffer;
}

void StreamingBuffer::Flush(UInt32 ctx_num)
{
	map_infos[ctx_num].mapped_data.Unmap();
	map_infos[ctx_num].curr_offset = 0;
}

void StreamingBuffer::Release(UInt32 ctx_num)
{
	if (!allow_persistent_map)
	{
		map_infos[ctx_num].mapped_data.Unmap();
	}
}

UInt32 StreamingBuffer::Allocate(UInt32 in_size, UInt32 ctx_num)
{
	auto& map_info = map_infos[ctx_num];
	if (map_info.curr_offset + in_size > buffer->GetBufferDesc().size)
	{
		Flush(ctx_num);
	}

	if (map_info.mapped_data == nullptr)
	{
		map_info.mapped_data.Map(buffer,ENUM_MAP_TYPE::Write, map_info.curr_offset == 0 ? ENUM_MAP_FLAG::Discard : ENUM_MAP_FLAG::NoOverwrite);
	}
	auto offset = map_info.curr_offset;
	map_info.curr_offset += in_size;
	return offset;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE

