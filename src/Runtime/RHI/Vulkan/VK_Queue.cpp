#include "VK_Queue.h"
#include <iostream>
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_CommandBuffer.h"

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

UInt32 VK_Queue::GetFamily() const
{
	return family_index;
}

UInt32 VK_Queue::GetQueueIndex() const
{
	return queue_index;
}

void VK_Queue::Submit(VK_CommandBuffer* command_list, UInt32 NumSignalSemaphores = 0, VkSemaphore* SignalSemaphores = nullptr)
{

}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE