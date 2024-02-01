#include"VK_Buffer.h"
#include "VK_Define.h"
#include "VK_Utils.h"
#include "VK_CommandBuffer.h"
#include "../../Core/ConstGlobals.h"
#include "VK_Memory.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

extern enum
{
	VK_NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS = 3,
};



VK_Buffer::VK_Buffer(VK_Device* in_device, const BufferDesc& in_buffer_desc) :Buffer(in_buffer_desc),
device(in_device)
{
	VkBufferCreateInfo buffer_create_info = {};
	GenerateBufferCreateInfo(buffer_create_info, in_buffer_desc);
	vkCreateBuffer(device->GetDevice(), &buffer_create_info, VULKAN_CPU_ALLOCATOR, &buffer);

	device->GetMemoryManager()->AllocateBufferMemory(allocation, buffer, 
		TranslateBufferTypeToVulkanAllocationFlags(buffer_desc.type), 0);

}

void VK_Buffer::GenerateBufferCreateInfo(VkBufferCreateInfo& buffer_create_info, const BufferDesc& desc)
{
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = desc.size;
	buffer_create_info.usage =VK_Utils::Translate_Buffer_usage_type_To_VulkanUsageFlag(desc.type); //VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
}

void VK_Buffer::Destroy()
{
	vkDestroyBuffer(device->GetDevice(), buffer, VULKAN_CPU_ALLOCATOR);
	buffer = VK_NULL_HANDLE;
	VK_MemoryManager* memory_manager = device->GetMemoryManager();
	memory_manager->FreeAllocation(allocation);
}

void* VK_Buffer::Map()
{
	if (lock_state == LockState::Locked)
	{
		CHECK_WITH_LOG(true, "RHI Error :  This buffer has been locked !")
		return nullptr;
	}
	lock_state = LockState::Locked;
	return allocation.GetMappedPointer(device);
}

void VK_Buffer::Unmap()
{
	lock_state = LockState::Unlocked;
}



ENUM_VulkanAllocationFlags VK_Buffer::TranslateBufferTypeToVulkanAllocationFlags(const ENUM_BUFFER_TYPE& buffer_usage)
{
	ENUM_VulkanAllocationFlags allocation_flags = ENUM_VulkanAllocationFlags::None;
	switch (buffer_usage)
	{
	case ENUM_BUFFER_TYPE::Index:
	{
		allocation_flags= allocation_flags| ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	case ENUM_BUFFER_TYPE::Vertex:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::Uniform:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind ;
		break;
	}
	case ENUM_BUFFER_TYPE::Staging:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::None:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	default:
		CHECK_WITH_LOG(true, "RHI Error :  Not support this buffer usage type !")
		break;
	}
	return allocation_flags;
}

VK_Buffer::~VK_Buffer()
{
	VK_MemoryManager* memory_manager = device->GetMemoryManager();
	memory_manager->FreeAllocation(allocation);
}

VkBuffer VK_Buffer::GetBuffer() CONST
{
	return buffer;
}

VK_StagingBufferManager::VK_StagingBufferManager(VK_Device* in_device) :device(in_device)
{

}

void VK_StagingBufferManager::ProcessPendingFree(Bool is_immediate, Bool is_free_to_os)
{
	Int ori_free_buffer_size = free_buffers.size();
	for (Int i = 0; i < pending_items_per_cmd_buffer.size(); i++)
	{
		PendingItemPerCmdBuffer& pending_item = pending_items_per_cmd_buffer[i];
		for(Int fence_index = 0; fence_index < pending_item.pending_items.size(); fence_index++)
		{
			PendingItemPerCmdBuffer::PendingItems& pending_item_per_fence = pending_item.pending_items[fence_index];
			if (is_immediate ||pending_item_per_fence.fence < pending_item.command_buffer->GetFenceSignaledCounter())
			{
				for (Int buffer_index = 0; buffer_index < pending_item_per_fence.buffer.size(); buffer_index++)
				{
					VK_Buffer* buffer = pending_item_per_fence.buffer[buffer_index];
					free_buffers.push_back({ buffer, g_frame_number_render_thread });
				}
				pending_item_per_fence.buffer.clear();
				pending_item.pending_items.erase(pending_item.pending_items.begin() + fence_index);
				fence_index--;
			}
		}
	}

	if (is_free_to_os)
	{
		for (Int i = 0; i < free_buffers.size(); i++)
		{
			if (free_buffers[i].frame_num + VK_NUM_FRAMES_TO_WAIT_BEFORE_RELEASING_TO_OS < g_frame_number_render_thread)
			{
				free_buffers[i].buffer->Destroy();
				delete free_buffers[i].buffer;
				free_buffers.erase(free_buffers.begin() + i);
				i--;
			}
		}
	}
}

void VK_StagingBufferManager::ReleaseStagingBuffer(VK_Buffer*& buffer, VK_CommandBuffer* command_buffer)
{
	if (command_buffer)
	{
		PendingItemPerCmdBuffer* ItemsForCmdBuffer = FindOrAddPendingItemPerCmdBuffer(command_buffer);
		PendingItemPerCmdBuffer::PendingItems* ItemsForFence = ItemsForCmdBuffer->FindOrAddItemsForFence(command_buffer->GetFenceSignaledCounter());
		ItemsForFence->buffer.push_back(buffer);
	}
	else
	{
		free_buffers.push_back({ buffer, g_frame_number_render_thread });
	}
	buffer = nullptr;
}

VK_Buffer* VK_StagingBufferManager::GetStagingBuffer(UInt64 size)
{

	for (auto& stagingbuffer : free_buffers)
	{
		if (stagingbuffer.buffer->GetBufferDesc().size == size)
		{
			//从free_buffers中移除，std::vector没有remove方法，所以用erase
		    free_buffers.erase(std::remove(free_buffers.begin(), free_buffers.end(), stagingbuffer), free_buffers.end());
			return stagingbuffer.buffer;
		}
	}

	
	BufferDesc staging_buffer_desc;

	staging_buffer_desc.size = size;
	staging_buffer_desc.stride = 4;
	staging_buffer_desc.type = ENUM_BUFFER_TYPE::Staging;
	VK_Buffer* staging_buffer = new VK_Buffer(device, staging_buffer_desc);
	using_buffers.push_back(staging_buffer);
	return staging_buffer;
}

VK_StagingBufferManager::PendingItemPerCmdBuffer* VK_StagingBufferManager::FindOrAddPendingItemPerCmdBuffer(VK_CommandBuffer* command_buffer)
{
	for (auto& pending_item : pending_items_per_cmd_buffer)
	{
		if (pending_item.command_buffer == command_buffer)
		{
			return &pending_item;
		}
	}
	PendingItemPerCmdBuffer new_pending_item;
	new_pending_item.command_buffer = command_buffer;
	pending_items_per_cmd_buffer.push_back(new_pending_item);
	return &pending_items_per_cmd_buffer.back();

}

VK_StagingBufferManager::~VK_StagingBufferManager()
{
	ProcessPendingFree(true, true);
}

inline VK_StagingBufferManager::PendingItemPerCmdBuffer::PendingItems* VK_StagingBufferManager::PendingItemPerCmdBuffer::FindOrAddItemsForFence(UInt64 fence)
{
	for (auto& pending_item : pending_items)
	{
		if (pending_item.fence == fence)
		{
			return &pending_item;
		}
	}
	PendingItems new_pending_item;
	new_pending_item.fence = fence;
	pending_items.push_back(new_pending_item);
	return &pending_items.back();
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE