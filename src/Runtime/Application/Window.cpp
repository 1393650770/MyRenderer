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

Window::Window(CONST String& in_title) : title(in_title)
{

	if (!glfwInit())
	{
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfw_window = glfwCreateWindow(width,height, title.c_str(),NULL, NULL);
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
	EThreadingMode mode = g_thread_mode;
	Bool use_rhi_thread = (mode >= EThreadingMode::RHIThread);
	Bool use_render_thread = (mode >= EThreadingMode::ThreeThread);

	if (use_rhi_thread) {
		RHIStartRHIThread();
	}
	if (use_render_thread) {
		// ThreeThread: Logic init on main, Render init on Render thread
		render->OnInit_Logic(this);
		frame_sync.StartRenderThread(render, viewport);
	} else {
		// Single / RHIThread: compat wrapper calls OnInit_Logic + OnInit_Render
		render->OnInit(this);
	}

	Int width = 0, height = 0;

	while (!glfwWindowShouldClose(glfw_window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();


		render->OnUpdate(deltaTime);

		switch (mode)
		{
		case EThreadingMode::Single:
		{
			Render::FrameContext ctx;
			ctx.deltaTime = deltaTime;
			ctx.viewport_width = this->width;
			ctx.viewport_height = this->height;
			render->OnPrepareFrameContext(ctx);
			render->OnPreRender(ctx);
			render->OnRender();
			render->OnPostRender(ctx);
			auto* cmd_list = RHIGetImmediateCommandList();
			viewport->Present(cmd_list, true, true);
			break;
		}

		case EThreadingMode::RHIThread:
		{
			Render::FrameContext ctx;
			ctx.deltaTime = deltaTime;
			ctx.viewport_width = this->width;
			ctx.viewport_height = this->height;
			render->OnPrepareFrameContext(ctx);
			render->OnPreRender(ctx);
			auto* cmd_list = RHIGetWriteCommandList();
			cmd_list->SetBypass(false);
			cmd_list->Begin();
			render->OnRender();
			render->OnPostRender(ctx);
			cmd_list->SetBypass(true);
			RHISwapCommandLists();

			while (!RHIIsReplayDone()) {
				std::this_thread::yield();
			}
			auto* present_cb = RHIGetRHICmdListForPresent();
			viewport->Present(present_cb, true, true);
			break;
		}

		case EThreadingMode::ThreeThread:
		{
			Render::FrameContext* ctx = frame_sync.AcquireWriteSlot();
			if (ctx)
			{
				ctx->deltaTime = deltaTime;
				ctx->viewport_width = this->width;
				ctx->viewport_height = this->height;
				ctx->frame_number = g_frame_number_render_thread;

				// ImGui CPU work (NewFrame+widgets+Render) in OnPrepareFrameContext
				render->OnPrepareFrameContext(*ctx);
			}

			frame_sync.SignalFrameReady();
			frame_sync.WaitFrameComplete();
			break;
		}
		}

		if (mode != EThreadingMode::ThreeThread)
		{
			RHIRenderEnd();
		}
		g_frame_number_render_thread = (g_frame_number_render_thread + 1) % g_max_frame_number;

		glfwSwapBuffers(glfw_window);

		glfwGetFramebufferSize(glfw_window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(glfw_window, &width, &height);
			glfwWaitEvents();
		}
		this->width = width;
		this->height = height;
		viewport->Resize(width, height);
	}

	if (use_render_thread) {
		frame_sync.StopRenderThread();
		render->OnShutdown_Logic();
	} else {
		render->OnShutdown();
	}
	if (use_rhi_thread) RHIStopRHIThread();

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
