#pragma once
#ifndef _WINDOW_
#define _WINDOW_
#include <GLFW/glfw3.h>
#include<memory>
namespace MXRender
{
	class Render;
	class Window
	{
	public:
		Window();
		virtual ~Window();
		void run(std::shared_ptr<Render> render);
	private:
		GLFWwindow* window;
		float deltaTime = 0.0f;	
		float lastFrame = 0.0f;
	};
}
#endif //_WINDOW_

