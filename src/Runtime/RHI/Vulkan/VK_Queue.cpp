#include "VK_Queue.h"
#include <iostream>
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_CommandBuffer.h"
#include "VK_Fence.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)



VK_Queue::VK_Queue(VK_Device* in_device, UInt32 in_family_index):family_index(in_family_index),device(in_device)
{
	vkGetDeviceQueue(device->GetDevice(),family_index,queue_index,&queue);
}

VK_Queue::~VK_Queue()
{
	
}

UInt32 VK_Queue::GetFamily() CONST
{
	return family_index;
}

UInt32 VK_Queue::GetQueueIndex() CONST
{
	return queue_index;
}

void VK_Queue::Submit(VK_CommandBuffer* command_list, UInt32 num_signal_semaphores, VkSemaphore* signal_semaphores, UInt32 num_wait_semaphores, VkSemaphore* wait_semaphores)
{
	CONST VkCommandBuffer command_buffers[]={command_list->GetCommandBuffer()};
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount=1;
	submitInfo.pCommandBuffers=command_buffers;
	submitInfo.signalSemaphoreCount=num_signal_semaphores;
	submitInfo.pSignalSemaphores=signal_semaphores;
	submitInfo.waitSemaphoreCount= num_wait_semaphores;
	submitInfo.pWaitSemaphores= wait_semaphores;
	VK_Fence* fence = command_list->GetFence();
	VkFence vk_fence = fence->GetFence();
	vkResetFences(device->GetDevice(), 1, &vk_fence);
	vkQueueSubmit(queue,1,&submitInfo, vk_fence);
	vkWaitForFences(device->GetDevice(), 1, &vk_fence, VK_TRUE, UINT64_MAX);
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE