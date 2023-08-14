#include "VK_CommandBuffer.h"
#include <iostream>
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_Define.h"

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


VK_CommandBuffer::~VK_CommandBuffer()
{

}

VK_CommandBuffer::VK_CommandBuffer(VK_Device* in_device, VK_CommandBufferPool* in_command_buffer_pool, Bool in_is_upload_only):device(in_device), owner_pool(in_command_buffer_pool), is_upload_only(in_is_upload_only)
{

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE