#include "VK_Viewport.h"
#include "VK_SwapChain.h"
#include "../GraphicsContext.h"
#include "VK_GraphicsContext.h"

namespace MXRender
{
	
	VK_Viewport::VK_Viewport(std::shared_ptr<GraphicsContext> Context, void* InWindowHandle, int InSizeX, int InSizeY, bool bInIsFullscreen)
	{
		VK_GraphicsContext* graphics_context= dynamic_cast<VK_GraphicsContext*>(Context.get());
		if (graphics_context)
		{
			int desired_num_backbuffers;
			std::vector<VkImage> image_array;
			ENUM_TEXTURE_FORMAT format= ENUM_TEXTURE_FORMAT::A8;
			swapchain=new VK_SwapChain(graphics_context->get_instance(),graphics_context->get_device(),InWindowHandle, format,InSizeX,InSizeY,bInIsFullscreen, &desired_num_backbuffers, image_array,0,nullptr);

		}
	}

	VK_Viewport::~VK_Viewport()
	{
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

}

