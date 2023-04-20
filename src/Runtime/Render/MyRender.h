#pragma once

#ifndef _MYRENDER_
#define _MYRENDER_
#include <memory>

struct GLFWwindow;
namespace MXRender { class RenderScene; }
namespace MXRender { class WindowUI; }

namespace MXRender { class VK_GraphicsContext; }

namespace MXRender
{
	class MyRender
	{
	public:
		MyRender();
		virtual ~MyRender();
		virtual void run(std::weak_ptr <VK_GraphicsContext> context, RenderScene* render_scene) = 0;
		virtual void init(std::weak_ptr <VK_GraphicsContext> context, GLFWwindow* window, WindowUI* window_ui) = 0;
	private:

	};

}
#endif // !_MYRENDER_

