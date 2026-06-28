#pragma once
#ifndef _VK_TEMPBLOCKALLOCATOR_
#define _VK_TEMPBLOCKALLOCATOR_
#include <vulkan/vulkan_core.h>
#include "Core/ConstDefine.h"
#include "VK_Memory.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

// Bump allocator for small per-frame uniform buffer allocations.
// Acquires large VkDeviceMemory blocks, maps them persistently, and sub-allocates
// with a simple atomic bump cursor. All blocks are reset at frame end (RenderEnd).
// Only used for uniform-buffer-compatible memory (HOST_VISIBLE | HOST_COHERENT).

MYRENDERER_BEGIN_CLASS(VK_TempBlockAllocator)

#pragma region METHOD
public:
	VK_TempBlockAllocator(VK_Device* in_device);
	VIRTUAL ~VK_TempBlockAllocator();

	// Allocate a sub-range from the current temp block. Returns the mapped pointer.
	// out_allocation receives a non-owning reference suitable for binding/buffer-address queries.
	Bool Alloc(UInt32 in_size, UInt32 in_alignment, VK_Allocation& out_allocation, void*& out_mapped_ptr);

	// Reset all block cursors. Safe because the current submit model is synchronous.
	void ResetAll();

	// Destroy all blocks and free device memory.
	void Destroy();

protected:
	// Create a new temp block of at least the requested size.
	bool AllocBlock(UInt32 min_size);

	VK_Device* device = nullptr;

	// Active block currently serving allocations.
	VkBuffer current_buffer = VK_NULL_HANDLE;
	VK_DeviceMemoryAllocation* current_memory = nullptr;
	uint8_t* current_mapped = nullptr;
	UInt32 current_offset = 0;
	UInt32 current_block_size = 0;

	static constexpr UInt32 InitialBlockSize = 4 * 1024 * 1024;   // 4 MB
	static constexpr UInt32 MaxBlockSize = 64 * 1024 * 1024;      // 64 MB
	static constexpr UInt32 BlockAlignment = 256;

	Vector<VkBuffer> all_buffers;
	Vector<VK_DeviceMemoryAllocation*> all_memories;

private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
