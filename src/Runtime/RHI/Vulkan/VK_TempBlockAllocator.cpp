#include "VK_TempBlockAllocator.h"
#include "VK_Device.h"
#include "VK_Define.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

VK_TempBlockAllocator::VK_TempBlockAllocator(VK_Device* in_device)
	: device(in_device)
{
}

VK_TempBlockAllocator::~VK_TempBlockAllocator()
{
	Destroy();
}

bool VK_TempBlockAllocator::AllocBlock(UInt32 min_size)
{
	UInt32 block_size = current_block_size == 0 ? InitialBlockSize : (std::min)(current_block_size * 2, MaxBlockSize);
	block_size = (std::max)(block_size, min_size);

	VK_DeviceMemoryManager* mem_mgr = device->GetDeviceMemoryManager();

	// Find a host-visible + coherent memory type.
	VkBufferUsageFlags buf_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (mem_mgr->HasUnifiedMemory())
	{
		mem_props |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	// Create the VkBuffer.
	VkBufferCreateInfo buf_info{};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.size = block_size;
	buf_info.usage = buf_usage;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer = VK_NULL_HANDLE;
	if (vkCreateBuffer(device->GetDevice(), &buf_info, VULKAN_CPU_ALLOCATOR, &buffer) != VK_SUCCESS)
		return false;

	// Query memory requirements and allocate.
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device->GetDevice(), buffer, &mem_reqs);

	UInt32 type_index = 0;
	if (!mem_mgr->GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, mem_props, &type_index))
	{
		vkDestroyBuffer(device->GetDevice(), buffer, VULKAN_CPU_ALLOCATOR);
		return false;
	}

	VK_DeviceMemoryAllocation* mem_alloc = mem_mgr->Alloc(block_size, type_index, nullptr, 1.0f, false);
	if (!mem_alloc)
	{
		vkDestroyBuffer(device->GetDevice(), buffer, VULKAN_CPU_ALLOCATOR);
		return false;
	}

	// Bind and map.
	vkBindBufferMemory(device->GetDevice(), buffer, mem_alloc->GetMemory(), 0);
	void* mapped = nullptr;
	vkMapMemory(device->GetDevice(), mem_alloc->GetMemory(), 0, block_size, 0, &mapped);

	current_buffer = buffer;
	current_memory = mem_alloc;
	current_mapped = static_cast<uint8_t*>(mapped);
	current_offset = 0;
	current_block_size = block_size;

	all_buffers.push_back(buffer);
	all_memories.push_back(mem_alloc);
	return true;
}

Bool VK_TempBlockAllocator::Alloc(UInt32 in_size, UInt32 in_alignment, VK_Allocation& out_allocation, void*& out_mapped_ptr)
{
	in_alignment = (std::max)(in_alignment, BlockAlignment);
	UInt32 aligned_offset = (current_offset + in_alignment - 1) & ~(in_alignment - 1);

	// If current block can't fit, allocate a new one.
	if (!current_buffer || aligned_offset + in_size > current_block_size)
	{
		if (!AllocBlock(in_size + in_alignment))
		{
			return false;
		}
		aligned_offset = 0;
	}

	out_mapped_ptr = current_mapped + aligned_offset;

	// Fill a non-owning allocation reference.
	out_allocation.Init(ENUM_VK_AllocationType::EVulkanAllocationTempBlockBuffer,
		ENUM_VK_AllocationMetaType::EVulkanAllocationMetaFrameTempBuffer,
		(UInt64)current_buffer, in_size, aligned_offset,
		0, 0, 0);
	out_allocation.SetOwnerShipFalse();

	current_offset = aligned_offset + in_size;
	return true;
}

void VK_TempBlockAllocator::ResetAll()
{
	current_offset = 0;
}

void VK_TempBlockAllocator::Destroy()
{
	VkDevice vk_device = device->GetDevice();
	VK_DeviceMemoryManager* mem_mgr = device->GetDeviceMemoryManager();
	for (VkBuffer buf : all_buffers)
	{
		if (buf != VK_NULL_HANDLE)
			vkDestroyBuffer(vk_device, buf, VULKAN_CPU_ALLOCATOR);
	}
	all_buffers.clear();
	for (VK_DeviceMemoryAllocation* mem : all_memories)
	{
		if (mem)
		{
			vkUnmapMemory(vk_device, mem->GetMemory());
			mem_mgr->Free(mem);
		}
	}
	all_memories.clear();
	current_buffer = VK_NULL_HANDLE;
	current_memory = nullptr;
	current_mapped = nullptr;
	current_offset = 0;
	current_block_size = 0;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
