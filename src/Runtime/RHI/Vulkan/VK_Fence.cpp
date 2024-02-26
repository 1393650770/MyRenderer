#include "VK_Fence.h"
#include <iostream>
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_CommandBuffer.h"
#include "VK_Define.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

VK_Fence::VK_Fence(VK_Device* in_device, VK_FenceManager* in_fence_manager, bool is_create_signaled):device(in_device),owner_fence_manager(in_fence_manager)
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	CHECK_WITH_LOG(vkCreateFence(device->GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS,
		"RHI Error: failed to create a fence!")

	ResetFence();
}

VK_Fence::~VK_Fence()
{
}

VkFence VK_Fence::GetFence() const
{
	return fence;
}

VK_FenceManager* VK_Fence::GetOwner() const
{
	return owner_fence_manager;
}

Bool VK_Fence::GetIsSignaled() CONST
{
	return state == ENUM_Fence_State::Signaled;
}
void VK_Fence::ResetFence()
{
	owner_fence_manager->ResetFence(this);
	state = ENUM_Fence_State::Signaled;
}

VK_FenceManager::VK_FenceManager(VK_Device* in_device): device(in_device)
{
}

VK_FenceManager::~VK_FenceManager()
{
}

VK_Fence* VK_FenceManager::GetOrCreateFence()
{
	if(free_fences.empty())
	{
		VK_Fence* new_fence= new VK_Fence(device,this,false);
		using_fences.push_back(new_fence);
		return new_fence;
	}
	
	VK_Fence* free_fence = 	free_fences.back();
	free_fences.pop_back();
	using_fences.push_back(free_fence);

	return free_fence;
	
}

void VK_FenceManager::DestroyManagerResource()
{
	CHECK_WITH_LOG(using_fences.size()!=0,"RHI Error: no all fences are done!");
	for(auto& fence: free_fences)
	{
		FreeFence(fence);
	}
	free_fences.clear();
}

void VK_FenceManager::FreeFence(VK_Fence*& fence)
{
	vkDestroyFence(device->GetDevice(),fence->fence,VULKAN_CPU_ALLOCATOR);
	fence->fence=VK_NULL_HANDLE;
	delete fence;
}

void VK_FenceManager::ResetFence(VK_Fence* fence)
{
	CHECK_WITH_LOG(vkResetFences(device->GetDevice(),1,&fence->fence),"RHI Error: fail to reset a fence!")
}

Bool VK_FenceManager::WaitForFence(VK_Fence* fence, UInt64 time_in_nanoseconds)
{
	auto find_result= std::find(using_fences.begin(),using_fences.end(),fence);
	CHECK_WITH_LOG (find_result==using_fences.end(),"RHI Error: fence is not in FenceManager !");
	VkResult result =vkWaitForFences(device->GetDevice(), 1, &(fence->fence), true, time_in_nanoseconds);
	switch (result)
	{
	case VK_SUCCESS:
		return true;
	case VK_TIMEOUT:
		break;
	default:
		break;
	}

	return false;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE