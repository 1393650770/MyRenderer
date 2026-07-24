#pragma once
#ifndef _UIINPUTBRIDGE_
#define _UIINPUTBRIDGE_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Input)
struct Key;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(UI)

/**
 * Abstract input bridge — backend-agnostic interface for routing
 * platform input (mouse, keyboard, touch) to the UI system.
 *
 * Application code uses ONLY this interface; the concrete backend
 * (e.g. RmlUIInputBridge) is created by UIManager and never exposed
 * to the application layer.
 *
 * Convention 12.1: No backend types leak through this interface.
 */
MYRENDERER_BEGIN_CLASS(UIInputBridge)
#pragma region METHOD
public:
	VIRTUAL ~UIInputBridge() MYDEFAULT;

	/// Mouse position in window coordinates (origin top-left, pixels).
	VIRTUAL void METHOD(ProcessMouseMove)(Int x, Int y) PURE;

	/// Mouse button press/release.  button: 0=left, 1=right, 2=middle.
	VIRTUAL void METHOD(ProcessMouseButton)(Int button, bool pressed) PURE;

	/// Mouse scroll delta.
	VIRTUAL void METHOD(ProcessMouseScroll)(Float32 dx, Float32 dy) PURE;

	/// Mouse cursor left the window.
	VIRTUAL void METHOD(ProcessMouseLeave)() PURE;

	/// Keyboard press/release using platform-neutral EKey.
	VIRTUAL void METHOD(ProcessKey)(CONST MXRender::Input::Key& key, bool pressed) PURE;

	/// Unicode text input from the OS character callback.
	VIRTUAL void METHOD(ProcessChar)(UInt32 codepoint) PURE;

	/// Returns true if the UI is currently capturing mouse interaction
	/// (e.g. a slider is being dragged, a text field is focused).
	VIRTUAL bool METHOD(IsMouseInteracting)() CONST PURE;

protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE // UI
MYRENDERER_END_NAMESPACE // MXRender

#endif // !_UIINPUTBRIDGE_
