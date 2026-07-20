#include "DesktopWindow.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)

DesktopWindow* DesktopWindow::s_instance = nullptr;

DesktopWindow::DesktopWindow(const String& title, UInt32 w, UInt32 h)
{
	s_instance = this;
	if (!glfwInit())
		return;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_glfw_window = glfwCreateWindow((int)w, (int)h, title.c_str(), nullptr, nullptr);
	if (!m_glfw_window)
	{
		glfwTerminate();
		return;
	}
}

DesktopWindow::~DesktopWindow()
{
	if (m_glfw_window)
	{
		glfwDestroyWindow(m_glfw_window);
		m_glfw_window = nullptr;
	}
	glfwTerminate();
	if (s_instance == this)
		s_instance = nullptr;
}

void DesktopWindow::GetFramebufferSize(Int& w, Int& h) CONST
{
	if (m_glfw_window)
		glfwGetFramebufferSize(m_glfw_window, &w, &h);
}

void DesktopWindow::PollEvents()
{
	glfwPollEvents();
	glfwSwapBuffers(m_glfw_window); // no-op for Vulkan
}

Bool DesktopWindow::ShouldClose() CONST
{
	return m_glfw_window ? (Bool)glfwWindowShouldClose(m_glfw_window) : true;
}

Float64 DesktopWindow::GetTime() CONST
{
	return (Float64)glfwGetTime();
}

Bool DesktopWindow::GetMouseButton(MouseButton btn) CONST
{
	if (!m_glfw_window) return false;
	int glfw_btn = GLFW_MOUSE_BUTTON_LEFT;
	switch (btn)
	{
	case MouseButton::Left:   glfw_btn = GLFW_MOUSE_BUTTON_LEFT;   break;
	case MouseButton::Middle: glfw_btn = GLFW_MOUSE_BUTTON_MIDDLE; break;
	case MouseButton::Right:  glfw_btn = GLFW_MOUSE_BUTTON_RIGHT;  break;
	}
	return glfwGetMouseButton(m_glfw_window, glfw_btn) == GLFW_PRESS;
}

void DesktopWindow::GetCursorPos(Float64& x, Float64& y) CONST
{
	if (m_glfw_window)
		glfwGetCursorPos(m_glfw_window, &x, &y);
}

void DesktopWindow::SetScrollCallback(void (*cb)(Float64, Float64))
{
	m_scroll_callback = cb;
	if (m_glfw_window)
		glfwSetScrollCallback(m_glfw_window,
			[](GLFWwindow*, double ox, double oy) {
				if (s_instance && s_instance->m_scroll_callback)
					s_instance->m_scroll_callback((Float64)ox, (Float64)oy);
			});
}

void DesktopWindow::OnScrollStatic(GLFWwindow*, Float64 ox, Float64 oy)
{
	if (s_instance && s_instance->m_scroll_callback)
		s_instance->m_scroll_callback(ox, oy);
}

// Platform factory — defined per-platform
UniquePtr<PlatformWindow> CreatePlatformWindow(const String& title, UInt32 w, UInt32 h, void* platform_data)
{
	return std::make_unique<DesktopWindow>(title, w, h);
}

MYRENDERER_END_NAMESPACE
