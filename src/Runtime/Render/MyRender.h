#pragma once

#ifndef _MYRENDER_
#define _MYRENDER_
#include <memory>
#include "vulkan/vulkan_core.h"
#include "GLFW/glfw3.h"
namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
	class MyRender
	{
	public:
		MyRender();
		virtual ~MyRender();
		virtual void run(std::weak_ptr <VK_GraphicsContext> context) = 0;
		virtual void init(std::weak_ptr <VK_GraphicsContext> context, GLFWwindow* window) = 0;
	private:

	};

}
#endif // !_MYRENDER_

