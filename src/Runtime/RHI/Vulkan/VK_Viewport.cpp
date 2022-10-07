#include "VK_Viewport.h"
#include "VK_Device.h"
#include "VK_SwapChain.h"
#include "../GraphicsContext.h"
#include "VK_GraphicsContext.h"
#include "VK_Resource.h"

namespace MXRender
{
	
	VK_Viewport::VK_Viewport(std::shared_ptr<GraphicsContext> Context, void* InWindowHandle, int InSizeX, int InSizeY, bool bInIsFullscreen)
	{
		VK_GraphicsContext* graphics_context= dynamic_cast<VK_GraphicsContext*>(Context.get());
		device=graphics_context->get_device();
		if (graphics_context)
		{
			int desired_num_backbuffers;
			ENUM_TEXTURE_FORMAT format= ENUM_TEXTURE_FORMAT::A8;
			VK_SwapChainRecreateInfo recreate_info;
			recreate_info.surface= graphics_context->get_surface();
			swapchain=new VK_SwapChain(graphics_context->get_instance(),graphics_context->get_device(),InWindowHandle, format,InSizeX,InSizeY,bInIsFullscreen, &desired_num_backbuffers, image_array,0, &recreate_info);
			
		}
	}

	VK_Viewport::~VK_Viewport()
	{
		for (int i = 0; i < image_array.size(); ++i)
		{
			texture_view_array[i].Destroy(device.lock());

		}
		delete swapchain;

	}



	void VK_Viewport::destroy_swapchain(VK_SwapChainRecreateInfo* RecreateInfo)
	{
		if (RecreateInfo)
		{
			swapchain->destroy(RecreateInfo);
		}
		else
		{
			delete swapchain;
			swapchain=nullptr;
		}
	}

	void VK_Viewport::destroy_image_view()
	{
		
	}

	void VK_Viewport::create_image_view_from_swapchain()
	{
		if (device.expired())
		{
			return;
		}
		texture_view_array.resize(image_array.size());
		for(int i=0;i< image_array.size();++i)
		{
			texture_view_array[i].Create(device.lock(),image_array[i], VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,ENUM_TEXTURE_FORMAT::A8,swapchain->get_image_format(), 0, 1, 0, 1);
			
		}
	}

	VK_SwapChain* VK_Viewport::get_swapchain()
	{
		return swapchain;
	}

	int VK_Viewport::get_image_num() const
	{
		return image_array.size();
	}

	std::vector<VkImage>& VK_Viewport::get_image_array()
	{
		return image_array;
	}

	std::vector<VK_TextureView>& VK_Viewport::get_image_view_array()
	{
		return texture_view_array;
	}

}

