#pragma once
#ifndef _BUFFERUTILS_
#define _BUFFERUTILS_
#include "Core/ConstDefine.h"
#include "RHI/RenderEnum.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Buffer;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

// Buffer creation / upload helpers encoding the reliable-upload rules learned
// in Fluid2D/Fluid3D/Ocean (see CLAUDE.md "RHI Gotchas"):
// - Buffers WITHOUT the Dynamic bit are device-local; their Map(Write) goes
//   through a staging + TRANSFER-queue copy with no cross-queue sync and is
//   NOT reliable. Initialize device-local data with a one-shot compute pass.
// - Buffers WITH the Dynamic bit are host-visible persistently-mapped;
//   Map/memcpy/Unmap is a direct write and is the reliable upload path.
MYRENDERER_BEGIN_CLASS(BufferUtils)
#pragma region METHOD
public:
	// Device-local storage buffer (GPU-produced data). in_extra_flags may add
	// usage bits such as ENUM_BUFFER_TYPE::Indirect or Dynamic.
	static RHI::Buffer* METHOD(CreateStorageBuffer)(UInt32 in_size_bytes, UInt32 in_stride, ENUM_BUFFER_TYPE in_extra_flags = ENUM_BUFFER_TYPE::None);
	// Host-visible persistently-mapped parameter buffer (Storage|Dynamic).
	// Declare it in GLSL as `readonly buffer`, NOT a uniform block.
	static RHI::Buffer* METHOD(CreateDynamicParamBuffer)(UInt32 in_size_bytes);
	// Map(Write)/memcpy/Unmap direct write. Only reliable for buffers carrying
	// the Dynamic (or Staging) bit - see class comment.
	static void METHOD(Upload)(RHI::Buffer* in_buffer, CONST void* in_data, UInt32 in_size_bytes, UInt32 in_dst_offset = 0);
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
#endif
