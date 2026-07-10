#include "Window.h"

#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "Render/RenderInterface.h"
#include "RHI/RenderCommandList.h"
#include "Render/Core/CommandQueue.h"
#include <limits>
#include <thread>
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
	Bool use_rhi_thread = g_enable_rhi_thread;

	if (use_rhi_thread) {
		RHIStartRHIThread();
	}

	Int width = 0, height = 0;
	render->OnInit(this);

	while (!glfwWindowShouldClose(glfw_window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		// Logic
		render->OnUpdate(deltaTime);

		if (use_rhi_thread) {
			// Record into write-CB, atomically swap to RHI thread
			auto* cmd_list = RHIGetWriteCommandList();
			cmd_list->SetBypass(false);
			cmd_list->Begin();
			render->OnRender();
			cmd_list->SetBypass(true);
			RHISwapCommandLists();
		} else {
			render->OnRender();
		}

		if (use_rhi_thread) {
			// Wait for RHI replay (rarely blocks)
			while (!RHIIsReplayDone()) {
				std::this_thread::yield();
			}
			auto* present_cb = RHIGetRHICmdListForPresent();
			viewport->Present(present_cb, true, true);
		} else {
			auto* cmd_list = RHIGetImmediateCommandList();
			viewport->Present(cmd_list, true, true);
		}
		RHIRenderEnd();
		g_frame_number_render_thread = (g_frame_number_render_thread + 1) % g_max_frame_number;

		glfwSwapBuffers(glfw_window);

		glfwGetFramebufferSize(glfw_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(glfw_window, &width, &height);
			glfwWaitEvents();
		}
		viewport->Resize(width, height);
	}
	if (use_rhi_thread) RHIStopRHIThread();
	render->OnShutdown();

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
