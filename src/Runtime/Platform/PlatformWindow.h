#pragma once
#ifndef _PLATFORM_WINDOW_
#define _PLATFORM_WINDOW_

#include "Core/ConstDefine.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)

// Platform-neutral input state (value type, zero RHI dependency).
MYRENDERER_BEGIN_STRUCT(TouchState)
public:
	static constexpr Int kMaxPointers = 10;
	Int pointer_count = 0;
	struct Pointer {
		Float32 x = 0.0f, y = 0.0f;
		Bool active = false;
		Int  id = 0;
	} pointers[kMaxPointers];
	Float32 pinch_distance = 0.0f;
MYRENDERER_END_STRUCT

enum class MouseButton : UInt8 { Left = 0, Middle = 1, Right = 2 };

// Abstract platform window. Desktop: GLFW. Android: ANativeWindow via android_native_app_glue.
// The RHI layer receives the native handle as void* (GetNativeHandle), already cross-platform.
MYRENDERER_BEGIN_CLASS(PlatformWindow)
#pragma region METHOD
public:
	VIRTUAL ~PlatformWindow() MYDEFAULT;

	// -- Window lifecycle --
	VIRTUAL void* GetNativeHandle() CONST PURE;
	VIRTUAL void  GetFramebufferSize(Int& w, Int& h) CONST PURE;
	VIRTUAL void  PollEvents() PURE;
	VIRTUAL Bool  ShouldClose() CONST PURE;
	VIRTUAL Float64 GetTime() CONST PURE;

	// -- Input (desktop: GLFW polling; mobile: AInputEvent-driven) --
	VIRTUAL Bool     GetMouseButton(MouseButton btn) CONST { return false; }
	VIRTUAL void     GetCursorPos(Float64& x, Float64& y) CONST {}
	VIRTUAL void     SetScrollCallback(void (*cb)(Float64, Float64)) {}
	VIRTUAL TouchState GetTouchState() CONST { return {}; }

	// -- Platform identity --
	VIRTUAL Bool IsMobile() CONST { return false; }
#pragma endregion
MYRENDERER_END_CLASS

// Factory: returns the platform-appropriate window implementation.
// platform_data: nullptr on desktop, android_app* on Android (opaque to caller).
UniquePtr<PlatformWindow> CreatePlatformWindow(const String& title, UInt32 w, UInt32 h,
                                               void* platform_data = nullptr);

MYRENDERER_END_NAMESPACE

#endif // _PLATFORM_WINDOW_
