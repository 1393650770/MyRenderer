#pragma once

#ifndef _MYRENDER_
#define _MYRENDER_
#include <memory>
#include "vulkan/vulkan_core.h"
namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
	class MyRender
	{
	public:
		MyRender();
		virtual ~MyRender();
		virtual void run(std::weak_ptr <VK_GraphicsContext> context, VkSwapchainKHR& swapchain) = 0;
		virtual void init() = 0;
	private:

	};

}
#endif // !_MYRENDER_

