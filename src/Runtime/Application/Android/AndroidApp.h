#pragma once
#ifndef _ANDROID_APP_
#define _ANDROID_APP_

#if PLATFORM_ANDROID

#include "Core/ConstDefine.h"
#include <android_native_app_glue.h>
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
class RenderInterface;

// Platform-neutral touch state (value type, zero RHI dependency).
MYRENDERER_BEGIN_STRUCT(TouchState)
public:
	static constexpr Int kMaxPointers = 10;
	Int pointer_count = 0;
	struct Pointer {
		Float32 x = 0.0f, y = 0.0f;
		Bool active = false;
		Int  id = 0;
	} pointers[kMaxPointers];
	Float32 pinch_distance = 0.0f; // distance between first two active pointers
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_NAMESPACE(Application)

// Minimal Android window wrapper using android_native_app_glue.
// Callback-style: the user provides on_init / on_frame / on_shutdown.
MYRENDERER_BEGIN_CLASS(AndroidApp)
#pragma region METHOD
public:
	AndroidApp(struct android_app* in_app);
	~AndroidApp();

	// Blocking: processes the Android event loop until APP_CMD_DESTROY.
	// in_on_init fires once after the window + swapchain are ready.
	// in_on_frame fires each frame (polled via ALooper).
	// in_on_shutdown fires when the window is being destroyed (before RHI shutdown).
	void METHOD(Run)(std::function<void()> in_on_init, std::function<void()> in_on_frame,
	                 std::function<void()> in_on_shutdown = {});

	// Queries
	void* METHOD(GetNativeWindow)() CONST { return native_window; }
	Int    METHOD(GetWidth)() CONST       { return width; }
	Int    METHOD(GetHeight)() CONST      { return height; }
	Bool   METHOD(IsWindowReady)() CONST  { return native_window != nullptr; }
	MXRender::RHI::Viewport* METHOD(GetViewport)() CONST { return viewport; }
	struct android_app* METHOD(GetApp)() CONST { return app; }

	// AndroidAssetManager for file I/O (used by ShaderLibrary on Android)
	struct AAssetManager* METHOD(GetAssetManager)() CONST;

	// Touch input state (updated each frame from AInputEvent motion events)
	TouchState METHOD(GetTouchState)() CONST { return touch_state; }

protected:
	static void METHOD(OnAppCmd)(struct android_app* in_app, int32_t cmd);
	static int32_t METHOD(OnInputEvent)(struct android_app* in_app, AInputEvent* event);
	void METHOD(HandleCmd)(int32_t cmd);
	void METHOD(HandleMotionEvent)(AInputEvent* event);
	void METHOD(InitRHI)();
	void METHOD(ShutdownRHI)();

private:
#pragma endregion

#pragma region MEMBER
protected:
	struct android_app* app = nullptr;
	void*  native_window = nullptr; // ANativeWindow*
	Int    width = 0, height = 0;
	Bool   has_focus = false;
	Bool   is_destroy_requested = false;

	TouchState touch_state; // updated each frame from motion events
	MXRender::RHI::Viewport* viewport = nullptr;
	std::function<void()> on_init;
	std::function<void()> on_frame;
	std::function<void()> on_shutdown;

	// C++→C bridge: store 'this' pointer so static callback can route back
	static AndroidApp* s_instance;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // PLATFORM_ANDROID
#endif // _ANDROID_APP_
