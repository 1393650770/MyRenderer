#include "VK_Viewport.h"
#include "VK_RenderRHI.h"
#include "VK_Device.h"
#include "VK_SwapChain.h"
#include "VK_Texture.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)



VK_Viewport::VK_Viewport(VulkanRHI* in_rhi, VK_Device* in_device, void* in_window_handle, UInt32 in_size_x, UInt32 in_size_y, Bool in_is_full_screen, ENUM_TEXTURE_FORMAT in_pixel_format): device(in_device), rhi(in_rhi), window_handle(in_window_handle), size_x(in_size_x), size_y(in_size_y), is_full_screen(in_is_full_screen),pixel_format(in_pixel_format)
{
	rhi->viewports.push_back(this);
	CreateSwapChain();
	//maybe need to make sure the instance is already created.

}

void VK_Viewport::CreateSwapChain(VK_SwapChainRecreateInfo* recreate_info)
{
	Vector<VkImage> images;
	swap_chain=new VK_SwapChain(rhi->instance,device,window_handle,pixel_format,size_x,size_y,
								is_full_screen,&acquired_image_index, images, lock_to_sync, recreate_info);
	
	texture_views.resize(images.size());

	for (Int i = 0; i < images.size(); ++i)
	{
		texture_views[i].Create(*device,images[i], VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, pixel_format, VK_FORMAT_UNDEFINED, 0, 1, 0, 1);
	}

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
