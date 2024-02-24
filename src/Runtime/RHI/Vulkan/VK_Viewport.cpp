#include "VK_Viewport.h"
#include "VK_RenderRHI.h"
#include "VK_Device.h"
#include "VK_SwapChain.h"
#include "VK_Texture.h"
#include "VK_Utils.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)



VK_Viewport::VK_Viewport(VulkanRHI* in_rhi, VK_Device* in_device, void* in_window_handle, UInt32 in_size_x, UInt32 in_size_y, Bool in_is_full_screen,CONST ENUM_TEXTURE_FORMAT& in_pixel_format): device(in_device), rhi(in_rhi), window_handle(in_window_handle), size_x(in_size_x), size_y(in_size_y), is_full_screen(in_is_full_screen),common_pixel_format(in_pixel_format)
{
	rhi->viewports.push_back(this);
	CreateSwapChain();
	//maybe need to make sure the instance is already created.

}

Vector<Texture*> VK_Viewport::GetCurrentBackBufferRTV()
{
	return Vector<Texture*>(back_buffer_rtvs.begin(), back_buffer_rtvs.end());
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

void VK_Viewport::CreateSwapChain(VK_SwapChainRecreateInfo* recreate_info)
{
	Vector<VkImage> images;
	swap_chain=new VK_SwapChain(rhi->instance,device,window_handle,common_pixel_format,size_x,size_y,
								is_full_screen,&acquired_image_index, vk_pixel_format,images, lock_to_sync, recreate_info);
	
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
	}
	TextureDesc dsv_desc;
	dsv_desc.clear_value = { 1.0f, 0 };
	dsv_desc.width = size_x;
	dsv_desc.height = size_y;
	dsv_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_DEPTH;
	dsv_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT;
	dsv_desc.format = ENUM_TEXTURE_FORMAT::D32;
	back_buffer_dsv = new VK_Texture(device, dsv_desc);
}

VK_Viewport::~VK_Viewport()
{
	if (swap_chain)
	{
		swap_chain->Destroy(nullptr);
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
	rhi->viewports.erase(std::remove(rhi->viewports.begin(), rhi->viewports.end(), this), rhi->viewports.end());
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
