#pragma once
#ifndef _RMLUIINPUTBRIDGE_
#define _RMLUIINPUTBRIDGE_

#include "Core/ConstDefine.h"
#include "Input/InputKeys.h"

namespace Rml {
class Context;
}

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

/**
 * Platform-neutral input bridge: InputSystem/PlatformWindow → RmlUI Context.
 *
 * Uses the engine's InputSystem for modifier state and EKey for key identifiers.
 * No GLFW or platform-specific types in the API — follows convention 14.2.
 *
 * v1: Polling-based mouse/scroll/keyboard.
 * v2: Add ProcessTouch for mobile (uses PlatformWindow::GetTouchState).
 */
MYRENDERER_BEGIN_CLASS(RmlUIInputBridge)

#pragma region METHOD
public:
	RmlUIInputBridge() MYDEFAULT;
	VIRTUAL ~RmlUIInputBridge() MYDEFAULT;

	void METHOD(SetContext)(Rml::Context* context);

	// Mouse (window coordinates, origin top-left)
	void METHOD(ProcessMouseMove)(Int x, Int y);
	void METHOD(ProcessMouseButton)(Int button, bool pressed);
	void METHOD(ProcessMouseScroll)(Float32 dx, Float32 dy);
	void METHOD(ProcessMouseLeave)();

	// Keyboard — uses platform-neutral EKey
	void METHOD(ProcessKey)(CONST MXRender::Input::Key& key, bool pressed);

	// Text input (Unicode codepoint from GLFW char callback)
	// Note: text input still requires the platform layer to forward
	// GLFW's char callback to this method. That forwarding lives in
	// DesktopWindow, not in Sample code.
	void METHOD(ProcessChar)(UInt32 codepoint);

	bool METHOD(IsMouseInteracting)() CONST;

protected:
private:
	Rml::Context* m_context = nullptr;

	Int  METHOD(BuildModifiers)() CONST;
	static Int METHOD(MapKey)(CONST MXRender::Input::Key& key);
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
