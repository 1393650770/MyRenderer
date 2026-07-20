#pragma once
#ifndef _ANDROID_WINDOW_
#define _ANDROID_WINDOW_

#if PLATFORM_ANDROID

#include "Platform/PlatformWindow.h"
#include <android_native_app_glue.h>
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(AndroidWindow, public PlatformWindow)
#pragma region METHOD
public:
	AndroidWindow(struct android_app* in_app);
	VIRTUAL ~AndroidWindow() OVERRIDE;

	// PlatformWindow overrides
	VIRTUAL void* GetNativeHandle() CONST OVERRIDE FINAL { return (void*)m_native_window; }
	VIRTUAL void  GetFramebufferSize(Int& w, Int& h) CONST OVERRIDE FINAL { w = m_width; h = m_height; }
	VIRTUAL void  PollEvents() OVERRIDE FINAL {}
	VIRTUAL Bool  ShouldClose() CONST OVERRIDE FINAL { return m_destroy_requested; }
	VIRTUAL Float64 GetTime() CONST OVERRIDE FINAL { return 0.0; } // Android: no GLFW time
	VIRTUAL TouchState GetTouchState() CONST OVERRIDE FINAL { return m_touch_state; }
	VIRTUAL Bool  IsMobile() CONST OVERRIDE FINAL { return true; }

	// Android-specific
	Bool   IsWindowReady() CONST { return m_native_window != nullptr; }
	MXRender::RHI::Viewport* GetViewport() CONST { return m_viewport; }
	struct android_app* GetApp() CONST { return m_app; }
	struct AAssetManager* GetAssetManager() CONST;

	// Event loop — blocking until APP_CMD_DESTROY
	void Run(std::function<void()> on_init, std::function<void()> on_frame,
	         std::function<void()> on_shutdown = {});

protected:
	static void OnAppCmd(struct android_app* in_app, int32_t cmd);
	static int32_t OnInputEvent(struct android_app* in_app, AInputEvent* event);
	void HandleCmd(int32_t cmd);
	void HandleMotionEvent(AInputEvent* event);
	void InitRHI();
	void ShutdownRHI();
private:
#pragma endregion

#pragma region MEMBER
protected:
	struct android_app* m_app = nullptr;
	void*  m_native_window = nullptr;
	Int    m_width = 0, m_height = 0;
	Bool   m_has_focus = false;
	Bool   m_destroy_requested = false;

	TouchState m_touch_state;
	MXRender::RHI::Viewport* m_viewport = nullptr;

	std::function<void()> m_on_init;
	std::function<void()> m_on_frame;
	std::function<void()> m_on_shutdown;

	static AndroidWindow* s_instance;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE

#endif // PLATFORM_ANDROID
#endif // _ANDROID_WINDOW_
