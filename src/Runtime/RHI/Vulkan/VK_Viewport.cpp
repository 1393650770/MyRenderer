#include "VK_Viewport.h"
#include "VK_RenderRHI.h"
#include "VK_Device.h"
#include "VK_SwapChain.h"
#include "VK_Texture.h"
#include "VK_Utils.h"
#include "VK_Queue.h"
#include "VK_CommandBuffer.h"
#include "VK_Fence.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)



VK_Viewport::VK_Viewport(VulkanRHI* in_rhi, VK_Device* in_device, void* in_window_handle, UInt32 in_size_x, UInt32 in_size_y, Bool in_is_full_screen,CONST ENUM_TEXTURE_FORMAT& in_pixel_format): device(in_device), rhi(in_rhi), window_handle(in_window_handle), size_x(in_size_x), size_y(in_size_y), is_full_screen(in_is_full_screen),common_pixel_format(in_pixel_format)
{
	rhi->viewports.push_back(this);
	CreateSwapChain();
	CreateSyncObjects();
	//maybe need to make sure the instance is already created.
}

Texture* VK_Viewport::GetCurrentBackBufferRTV()
{
	return back_buffer_rtvs[acquired_image_index];
}
Texture* VK_Viewport::GetCurrentBackBufferDSV() 
{
	return back_buffer_dsv;
}
Vector<UInt32> VK_Viewport::GetViewportSize() CONST
{
	return { size_x, size_y };
}

UInt32 VK_Viewport::GetViewportSizeWidth() CONST
{
	return size_x;
}

UInt32 VK_Viewport::GetViewportSizeHeight() CONST
{
	return size_y;
}

void VK_Viewport::Resize(UInt32 in_width, UInt32 in_height)
{
	size_x = in_width;
	size_y = in_height;
	VK_SwapChainRecreateInfo recreate_info;
	recreate_info.swapchain = swap_chain->GetSwapchain();
	recreate_info.surface = swap_chain->GetSurface();
	CreateSwapChain(&recreate_info);
}

void VK_Viewport::Present(MXRender::RHI::CommandList* in_cmd_list, bool is_present, bool is_lock_to_vsync)
{
	VK_Queue* submit_queue = device->GetQueue(ENUM_QUEUE_TYPE::GRAPHICS);
	VK_Queue* present_queue = device->GetQueue(ENUM_QUEUE_TYPE::PRESENT);
	CHECK_WITH_LOG(submit_queue == nullptr || present_queue == nullptr, "RHI Error : can not get free queue!");
	PresentInternal(STATIC_CAST(in_cmd_list, VK_CommandBuffer), submit_queue, present_queue, is_lock_to_vsync);
}

void VK_Viewport::PresentInternal(VK_CommandBuffer* in_cmd_list, VK_Queue* submit_queue, VK_Queue* present_queue, bool is_lock_to_vsync)
{
	//Submit
	submit_queue->Submit(in_cmd_list, 1, &(sumit_signal_semaphore[0]), 1, &image_acquired_semaphore);
	//Present
	if (present_queue&&!is_minimized)
	{
		VK_CommandBuffer* temp_commandbuffer= device->GetCommandBufferManager()->GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE::GRAPHICS);
		temp_commandbuffer->Begin();
		temp_commandbuffer->TransitionTextureState(GetCurrentBackBufferRTV(), ENUM_RESOURCE_STATE::Present);
		submit_queue->Submit(temp_commandbuffer, 1, &(sumit_signal_semaphore[1]), 1, &(sumit_signal_semaphore[0]));
		VkResult result = swap_chain->PresentInternal(present_queue->GetQueue(),
			(sumit_signal_semaphore[1]),
			acquired_image_index,
			is_lock_to_vsync);
		in_cmd_list->command_state = VK_CommandBuffer::EState::NeedReset;
		temp_commandbuffer->command_state = VK_CommandBuffer::EState::NeedReset;
		device->GetCommandBufferManager()->ReleaseCommandBuffer(temp_commandbuffer);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ) {
			VK_SwapChainRecreateInfo recreate_info;
			recreate_info.swapchain = swap_chain->GetSwapchain();
			recreate_info.surface = swap_chain->GetSurface();
			CreateSwapChain(&recreate_info);
		}
		else if (result != VK_SUCCESS) {
			CHECK_WITH_LOG (true, "RHI Error: failed to present swap chain image!");
		}
	}
		
	if (submit_queue != present_queue)
	{
		vkQueueWaitIdle(present_queue->GetQueue());
	}
	if (swap_chain->TryGetNextImageIndex(image_acquired_semaphore, acquired_image_index) == false)
	{
		VK_SwapChainRecreateInfo recreate_info;
		recreate_info.swapchain = swap_chain->GetSwapchain();
		recreate_info.surface = swap_chain->GetSurface();
		CreateSwapChain(&recreate_info);
	}
}

void VK_Viewport::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext = nullptr;
	for (Int i = 0; i < 2; i++)
	{
		CHECK_WITH_LOG(vkCreateSemaphore(device->GetDevice(), &semaphore_create_info, nullptr, &(sumit_signal_semaphore[i])) != VK_SUCCESS, "RHI Error : failed to create semaphore!");
	}
}

void VK_Viewport::DestroySwapChain(VK_SwapChainRecreateInfo* recreate_info)
{
	if (swap_chain)
	{
		swap_chain->Destroy(recreate_info);
		delete swap_chain;
		swap_chain = nullptr;
	}
	for (Int i = 0; i < back_buffer_rtvs.size(); ++i)
	{
		delete back_buffer_rtvs[i];
	}
	if (back_buffer_dsv)
	{
		delete back_buffer_dsv;
	}
	back_buffer_rtvs.clear();
	texture_views.clear();
}

void VK_Viewport::CreateSwapChain(VK_SwapChainRecreateInfo* recreate_info)
{
	Vector<VkImage> images;
	UInt32 image_count = 0;
	swap_chain=new VK_SwapChain(rhi->instance,device,window_handle,common_pixel_format,size_x,size_y,
								is_full_screen,&image_count, vk_pixel_format,images, lock_to_sync, recreate_info);
	size_x = swap_chain->GetExtent2D().width;
	size_y = swap_chain->GetExtent2D().height;
	is_minimized = size_x == 0 || size_y == 0;

	texture_views.resize(images.size());
	common_pixel_format = VK_Utils::Translate_Vulkan_Texture_Format_To_Common(vk_pixel_format);
	for (Int i = 0; i < images.size(); ++i)
	{
		texture_views[i].Create(*device,images[i], VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, vk_pixel_format, 0, 1, 0, 1);
	}
	back_buffer_rtvs.resize(images.size());
	TextureDesc rtv_desc;
	rtv_desc.clear_value = { 0.0f, 0.0f, 0.0f, 1.0f };
	rtv_desc.width = size_x;
	rtv_desc.height = size_y;
	rtv_desc.format = common_pixel_format;
	rtv_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
	rtv_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_COLOR_ATTACHMENT;
	for (Int i = 0; i < images.size(); ++i)
	{
		back_buffer_rtvs[i] = new VK_Texture(device, texture_views[i], rtv_desc);
		rhi->GetImmediateCommandList()->TransitionTextureState(back_buffer_rtvs[i], ENUM_RESOURCE_STATE::RenderTarget);
	}
	TextureDesc dsv_desc;
	dsv_desc.clear_value = { 1.0f, 0 };
	dsv_desc.width = size_x;
	dsv_desc.height = size_y;
	dsv_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH;
	dsv_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT;
	dsv_desc.format = ENUM_TEXTURE_FORMAT::D32;
	back_buffer_dsv = new VK_Texture(device, dsv_desc);
	rhi->GetImmediateCommandList()->TransitionTextureState(back_buffer_dsv, ENUM_RESOURCE_STATE::DepthWrite);
	if(acquired_image_index>image_count)
		swap_chain->TryGetNextImageIndex(image_acquired_semaphore, acquired_image_index);
}

VK_Viewport::~VK_Viewport()
{
	if (swap_chain)
	{
		delete swap_chain;
		swap_chain = nullptr;
	}
	for (Int i = 0; i < 2; i++)
	{
		vkDestroySemaphore(device->GetDevice(), sumit_signal_semaphore[i], nullptr);
		sumit_signal_semaphore[i] = VK_NULL_HANDLE;
	}
	for (Int i = 0; i < texture_views.size(); ++i)
	{
		texture_views[i].Destroy(*device);
	}
	/*
	for (Int i = 0; i < back_buffer_rtvs.size(); ++i)
	{
		delete back_buffer_rtvs[i];
	}
	*/
	if (back_buffer_dsv)
	{
		delete back_buffer_dsv;
	}
	back_buffer_rtvs.clear();
	texture_views.clear();
	rhi->viewports.erase(std::remove(rhi->viewports.begin(), rhi->viewports.end(), this), rhi->viewports.end());
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
