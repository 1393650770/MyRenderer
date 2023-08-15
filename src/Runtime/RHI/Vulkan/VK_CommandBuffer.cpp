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

}

VK_CommandBuffer* VK_CommandBufferPool::GetOrCreateCommandBuffer(Bool is_upload_only)
{
	VK_CommandBuffer* cmd_buffer=nullptr;


	return cmd_buffer;
}

void VK_CommandBufferPool::Init(UInt32 queue_family_index)
{
	VkCommandPoolCreateInfo command_pool_create_info;
	command_pool_create_info.queueFamilyIndex = queue_family_index;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	CHECK_WITH_LOG(vkCreateCommandPool(device->GetDevice(), &command_pool_create_info, VULKAN_CPU_ALLOCATOR, &command_pool) != VK_SUCCESS, "RHI Error: Create CommandPool Error");
}

VkCommandPool VK_CommandBufferPool::GetPool() const
{
	return command_pool;
}


VK_CommandBuffer::~VK_CommandBuffer()
{
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
	CHECK_WITH_LOG(command_buffer!=VK_NULL_HANDLE,"RHI Error: command buffer is nullptr when freeing commandbuffer!");
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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE