#include "Window.h"
#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "Render/RenderInterface.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)
/*
static void on_window_size_callback(GLFWwindow* window, int width, int height)
{
	Singleton<MXRender::DefaultSetting>::get_instance().height = height;
	Singleton<MXRender::DefaultSetting>::get_instance().width = width;
};
*/


Window::Window()
{

	if (!glfwInit())
	{
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width,height, "MyRender", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return;
	}
	glfwSetWindowUserPointer(window, this);

	//glfwSetWindowSizeCallback(window, on_window_size_callback);


 }

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::Run(RenderInterface* render)
{
	render->BeginRender();
	while (!glfwWindowShouldClose(window))
    {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		glfwPollEvents();
		render->BeginFrame();
		render->OnFrame();
		render->EndFrame();
        glfwSwapBuffers(window);
    }
	render->EndRender();
}

GLFWwindow* Window::GetWindow() CONST
{
    return window;
}

void Window::InitWindow()
{

	RHIInit();
	viewport_rhi = RHICreateViewport((void*)window,width,height, is_full_screen);

}

MXRender::RHI::Viewport* Window::GetViewport() CONST
{
	return viewport_rhi;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
