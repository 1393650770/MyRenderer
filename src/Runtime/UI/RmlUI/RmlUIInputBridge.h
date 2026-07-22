#pragma once
#ifndef _RMLUIINPUTBRIDGE_
#define _RMLUIINPUTBRIDGE_

#include "Core/ConstDefine.h"

// Forward declare Rml types (no header dependency in this header)
namespace Rml {
class Context;
namespace Input { enum class KeyIdentifier; }
}

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

/**
 * GLFW input → RmlUI Context event bridge.
 *
 * Owns the key code mapping table (GLFW → Rml::KeyIdentifier).
 * Call the Process* methods in the main loop after glfwPollEvents().
 *
 * v1: Direct GLFW polling (works in Single thread mode).
 * v2: Multi-thread support via buffered input queue.
 */
MYRENDERER_BEGIN_CLASS(RmlUIInputBridge)

#pragma region METHOD
public:
	RmlUIInputBridge() MYDEFAULT;
	VIRTUAL ~RmlUIInputBridge() MYDEFAULT;

	/// Set the RmlUI context to forward events to.
	void METHOD(SetContext)(Rml::Context* context);

	/// Process mouse movement.
	void METHOD(ProcessMouseMove)(Int32 x, Int32 y);

	/// Process mouse button (0=left, 1=right, 2=middle).
	void METHOD(ProcessMouseButton)(Int32 button, bool pressed);

	/// Process mouse scroll.
	void METHOD(ProcessMouseScroll)(Float32 dx, Float32 dy);

	/// Process key press/release (GLFW key code).
	void METHOD(ProcessKey)(Int32 glfw_key, bool pressed);

	/// Process a Unicode character input.
	void METHOD(ProcessChar)(UInt32 codepoint);

	/// Notify that mouse left the window.
	void METHOD(ProcessMouseLeave)();

	/// Returns true if the UI is currently capturing mouse input.
	bool METHOD(IsMouseInteracting)() CONST;

	/// Set input priority: true = game UI first, false = ImGui editor first.
	void METHOD(SetInputPriority)(bool game_first);

protected:
private:
	Rml::Context* m_context = nullptr;
	bool m_game_first = true;
	Int32 m_modifier_state = 0;

	/// Build KeyModifier state from current key states.
	Int32 METHOD(BuildModifiers)() CONST;

	/// Map GLFW key code to Rml::KeyIdentifier.
	static Int32 METHOD(MapKey)(Int32 glfw_key);
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

#endif // !_RMLUIINPUTBRIDGE_
