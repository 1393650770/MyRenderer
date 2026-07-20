#pragma once
#ifndef _DESKTOP_WINDOW_
#define _DESKTOP_WINDOW_

#include "Platform/PlatformWindow.h"

struct GLFWwindow;

MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(DesktopWindow, public PlatformWindow)
#pragma region METHOD
public:
	DesktopWindow(const String& title, UInt32 w, UInt32 h);
	VIRTUAL ~DesktopWindow() OVERRIDE;

	// PlatformWindow overrides
	VIRTUAL void* GetNativeHandle() CONST OVERRIDE FINAL { return (void*)m_glfw_window; }
	VIRTUAL void  GetFramebufferSize(Int& w, Int& h) CONST OVERRIDE FINAL;
	VIRTUAL void  PollEvents() OVERRIDE FINAL;
	VIRTUAL Bool  ShouldClose() CONST OVERRIDE FINAL;
	VIRTUAL Float64 GetTime() CONST OVERRIDE FINAL;

	VIRTUAL Bool  GetMouseButton(MouseButton btn) CONST OVERRIDE FINAL;
	VIRTUAL void  GetCursorPos(Float64& x, Float64& y) CONST OVERRIDE FINAL;
	VIRTUAL void  SetScrollCallback(void (*cb)(Float64, Float64)) OVERRIDE FINAL;

protected:
	static void METHOD(OnScrollStatic)(GLFWwindow* w, Float64 ox, Float64 oy);
private:
#pragma endregion

#pragma region MEMBER
protected:
	GLFWwindow* m_glfw_window = nullptr;
	void (*m_scroll_callback)(Float64, Float64) = nullptr;
	// DesktopWindow owns the GLFW lifetime - only one instance per process
	static DesktopWindow* s_instance;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE

#endif
