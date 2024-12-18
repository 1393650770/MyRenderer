#include "Window.h"

#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "Render/RenderInterface.h"
#include "RHI/RenderCommandList.h"
#include <limits>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

Window::Window()
{

	if (!glfwInit())
	{
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfw_window = glfwCreateWindow(width,height, "MXRender",NULL, NULL);
	if (!glfw_window)
	{
		glfwTerminate();
		return;
	}
	glfwSetWindowUserPointer(glfw_window, this);

	//glfwSetWindowSizeCallback(window, on_window_size_callback);

 }

Window::~Window()
{
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

void Window::Run(RenderInterface* render)
{
	Int width = 0;
	Int height = 0;
	render->BeginRender();
	while (!glfwWindowShouldClose(glfw_window))
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
		RHIRenderEnd();
		g_frame_number_render_thread = (g_frame_number_render_thread + 1) % g_max_frame_number;
        glfwSwapBuffers(glfw_window);


		glfwGetFramebufferSize(glfw_window, &width, &height);
		while (width == 0 || height == 0) // minimized 0,0, pause for now
		{
			glfwGetFramebufferSize(glfw_window, &width, &height);
			glfwWaitEvents();
		}
		viewport->Resize(width, height);

    }
	render->EndRender();

	delete viewport;
	viewport = nullptr;
	RHIShutdown();
}

GLFWwindow* Window::GetWindow() CONST
{
    return glfw_window;
}

void Window::InitWindow()
{

	RHIInit();
	viewport = RHICreateViewport((void*)glfw_window, width, height, is_full_screen);
}

MXRender::RHI::Viewport* Window::GetViewport() CONST
{
	return viewport;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
