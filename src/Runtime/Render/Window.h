#pragma once
#ifndef _WINDOW_
#define _WINDOW_
#include <GLFW/glfw3.h>
#include<memory>
namespace MXRender
{
	class MyRender;
	class Window
	{
	public:
		Window();
		virtual ~Window();
		void run(std::shared_ptr<MyRender> render);
		GLFWwindow* GetWindow() const;
	private:
		GLFWwindow* window;
		float deltaTime = 0.0f;	
		float lastFrame = 0.0f;
	};
}
#endif //_WINDOW_

