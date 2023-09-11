#include "VK_CommandBuffer.h"
#include "VK_Device.h"
#include "VK_Define.h"
#include "VK_Fence.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


VK_CommandBufferPool::VK_CommandBufferPool(VK_Device* in_device, VK_CommandBufferManager& in_cmd_buffer_manager):device(in_device),cmd_buffer_manager(in_cmd_buffer_manager)
{

}

VK_CommandBufferPool::~VK_CommandBufferPool()
{
	for (UInt32 index = 0; index < cmd_buffers.size(); ++index)
	{
		delete cmd_buffers[index];
	}
	for (UInt32 index = 0; index < free_cmd_buffers.size(); ++index)
	{
		delete free_cmd_buffers[index];
	}
	vkDestroyCommandPool(device->GetDevice(), command_pool, VULKAN_CPU_ALLOCATOR);
	command_pool=VK_NULL_HANDLE;
}

VK_CommandBuffer* VK_CommandBufferPool::GetOrCreateCommandBuffer(Bool is_upload_only)
{
	for (UInt32 index = free_cmd_buffers.size()-1; index>=0;--index)
	{
		VK_CommandBuffer* cmd_buffer = free_cmd_buffers[index];
		if (cmd_buffer->is_upload_only == is_upload_only)
		{
			free_cmd_buffers.erase(free_cmd_buffers.begin() + index);
			cmd_buffer->Allocate();
			cmd_buffers.push_back(cmd_buffer);
			return cmd_buffer;
		}
	}
	VK_CommandBuffer* cmd_buffer = new VK_CommandBuffer(device, this, is_upload_only);
	cmd_buffers.push_back(cmd_buffer);
	return cmd_buffer;
}

void VK_CommandBufferPool::Init(UInt32 queue_family_index)
{
	VkCommandPoolCreateInfo command_pool_create_info;
	command_pool_create_info.queueFamilyIndex = queue_family_index;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CHECK_WITH_LOG(vkCreateCommandPool(device->GetDevice(), &command_pool_create_info, VULKAN_CPU_ALLOCATOR, &command_pool) != VK_SUCCESS, "RHI Error: Create CommandPool Error");
}

void VK_CommandBufferPool::FreeUnusedCommandBuffer(VK_Queue* queue)
{
	for (UInt32 index = cmd_buffers.size() - 1; index >= 0; --index)
	{
		VK_CommandBuffer* cmd_buffer = cmd_buffers[index];
		if (cmd_buffer->GetFence()->GetIsSignaled())
		{
			cmd_buffer->Free();
			free_cmd_buffers.push_back(cmd_buffer);
			cmd_buffers.erase(cmd_buffers.begin() + index);
		}
	}
}

VkCommandPool VK_CommandBufferPool::GetPool() const
{
	return command_pool;
}


VK_CommandBuffer::~VK_CommandBuffer()
{
	if (false)
	{
		// Wait 33ms
		UInt64 wait_seconds = 33 * 1000 * 1000LL;
		device->GetFenceManager()->WaitForFence(fence, wait_seconds);
	}
	if(command_buffer!=VK_NULL_HANDLE)
	{ 
		Free();
		command_buffer=VK_NULL_HANDLE;
	}
}

VkCommandBuffer VK_CommandBuffer::GetCommandBuffer() CONST
{
	return command_buffer;
}

VK_Fence* VK_CommandBuffer::GetFence() const
{
	return fence;
}

void VK_CommandBuffer::Allocate()
{
	VkCommandBufferAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandBufferCount =1;
	allocate_info.level= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandPool=owner_pool->GetPool();
	CHECK_WITH_LOG(vkAllocateCommandBuffers(device->GetDevice(), &allocate_info, &command_buffer) != VK_SUCCESS,
		"RHI Error: failed to allocate command buffers!");
}

void VK_CommandBuffer::Free()
{
	CHECK_WITH_LOG(command_buffer==VK_NULL_HANDLE,"RHI Error: command buffer is nullptr when freeing commandbuffer!");
	vkFreeCommandBuffers(device->GetDevice(), owner_pool->GetPool(), 1, &command_buffer);
	command_buffer=VK_NULL_HANDLE;
	
}

VK_CommandBuffer::VK_CommandBuffer(VK_Device* in_device, VK_CommandBufferPool* in_command_buffer_pool, Bool in_is_upload_only):
		device(in_device),
		owner_pool(in_command_buffer_pool),
		is_upload_only(in_is_upload_only)
{
	Allocate();
	fence=device->GetFenceManager()->GetOrCreateFence();
}

Bool VK_CommandBuffer::WaitForFence(float time_in_seconds_to_wait)
{
	return device->GetFenceManager()->WaitForFence(fence, time_in_seconds_to_wait);
}

VK_CommandBufferManager::~VK_CommandBufferManager()
{
	for (UInt32 index = 0; index < graphic_cmd_buffers.size(); ++index)
	{
		delete graphic_cmd_buffers[index];
	}
	for (UInt32 index = 0; index < compute_cmd_buffers.size(); ++index)
	{
		delete compute_cmd_buffers[index];
	}
	for (UInt32 index = 0; index < transfer_cmd_buffers.size(); ++index)
	{
		delete transfer_cmd_buffers[index];
	}
}

VK_CommandBufferManager::VK_CommandBufferManager(VK_Device* in_device):device(in_device)
{
	graphic_cmd_pools.emplace_back(in_device,*this);
	graphic_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().graphics_family.value());
	compute_cmd_pools.emplace_back(in_device, *this);
	compute_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().compute_family.value());
	transfer_cmd_pools.emplace_back(in_device, *this);
	transfer_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().transfer_family.value());

	graphic_cmd_buffers.push_back(std::move(graphic_cmd_pools[0].GetOrCreateCommandBuffer(false)));
	compute_cmd_buffers.push_back(std::move(compute_cmd_pools[0].GetOrCreateCommandBuffer(false)));
	transfer_cmd_buffers.push_back(std::move(transfer_cmd_pools[0].GetOrCreateCommandBuffer(false)));

}

VK_CommandBuffer* VK_CommandBufferManager::GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE queue_type)
{
	switch (queue_type)
	{
	case ENUM_QUEUE_TYPE::GRAPHICS :
		return graphic_cmd_pools[0].GetOrCreateCommandBuffer(false);
	case ENUM_QUEUE_TYPE::COMPUTE:
		return compute_cmd_pools[0].GetOrCreateCommandBuffer(false);
	case ENUM_QUEUE_TYPE::TRANSFER:
		return transfer_cmd_pools[0].GetOrCreateCommandBuffer(true);
	default:
		return nullptr;
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE