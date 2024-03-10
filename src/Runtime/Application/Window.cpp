#include "Window.h"
#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "Render/RenderInterface.h"
#include "RHI/RenderCommandList.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

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
		MXRender::RHI::CommandList* cmd_list = RHIGetImmediateCommandList();

		render->BeginFrame();
		render->OnFrame();
		render->EndFrame();
		
		viewport->Present(cmd_list, true, true);

        glfwSwapBuffers(window);
    }
	render->EndRender();

	delete viewport;
	viewport = nullptr;
	RHIShutdown();
}

GLFWwindow* Window::GetWindow() CONST
{
    return window;
}

void Window::InitWindow()
{

	RHIInit();
	viewport = RHICreateViewport((void*)window,width,height, is_full_screen);

}

MXRender::RHI::Viewport* Window::GetViewport() CONST
{
	return viewport;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
