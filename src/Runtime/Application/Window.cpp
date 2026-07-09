#include "Window.h"

#include<iostream>
#include <memory>
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "Render/RenderInterface.h"
#include "RHI/RenderCommandList.h"
#include "Render/Core/CommandQueue.h"
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
	Render::CommandQueue render_queue;
	Render::CommandQueue rhi_queue;
	Bool use_rhi_thread = g_enable_rhi_thread;

	// RHIWorker fiber: consumes rhi_queue asynchronously on TaskScheduler
	struct RHIWorker {
		MT_DECLARE_TASK(RHIWorker, MT::StackRequirements::STANDARD, MT::TaskPriority::NORMAL, MT::Color::Green);
		Render::CommandQueue* queue;
		void Do(MT::FiberContext&) {
			while (queue->WaitAndFlush()) {}  // Block until Shutdown
		}
	};
	RHIWorker rhi_worker{ &rhi_queue };

	if (use_rhi_thread) {
		rhi_queue.SetBypass(false);
		scheduler.RunAsync(MT::TaskGroup::Default(), &rhi_worker, 1);
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

		// Render: record commands (bypass=false) or execute directly (bypass=true)
		auto* cmd_list = RHIGetImmediateCommandList();
		if (use_rhi_thread) cmd_list->SetBypass(false);
		render->OnRender();
		if (use_rhi_thread) {
			cmd_list->SetBypass(true);
			rhi_queue.Enqueue([cmd_list]() { cmd_list->Replay(); });
		}

		// Wait for RHI replay before Present (single-thread fallback: Flush)
		if (use_rhi_thread) rhi_queue.WaitAndFlush();  // TODO: replace with fence when fully async

		// Present
		viewport->Present(cmd_list, true, true);
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
	if (use_rhi_thread) rhi_queue.Shutdown();
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
