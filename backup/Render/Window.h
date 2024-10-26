#pragma once
#ifndef _WINDOW_
#define _WINDOW_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "DeferRender.h"

#include <array>
#include <functional>
#include <vector>
#include<memory>

namespace MXRender { class RenderScene; }

namespace MXRender
{
	class MyRender;
	class Window
	{
	private:
		GLFWwindow* window = nullptr;
		float deltaTime = 0.0f;
		float lastFrame = 0.0f;

	protected:



	public:
		Window();
		virtual ~Window();
		void run(std::shared_ptr<MyRender> render);
		GLFWwindow* GetWindow() const;


	};
}
#endif //_WINDOW_

