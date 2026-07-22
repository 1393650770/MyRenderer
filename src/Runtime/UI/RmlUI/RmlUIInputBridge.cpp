#include "RmlUIInputBridge.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Input.h>
#include <GLFW/glfw3.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

void RmlUIInputBridge::SetContext(Rml::Context* context)
{
	m_context = context;
}

void RmlUIInputBridge::ProcessMouseMove(Int32 x, Int32 y)
{
	if (!m_context) return;
	m_context->ProcessMouseMove(x, y, BuildModifiers());
}

void RmlUIInputBridge::ProcessMouseButton(Int32 button, bool pressed)
{
	if (!m_context) return;
	int mods = BuildModifiers();
	if (pressed)
		m_context->ProcessMouseButtonDown(button, mods);
	else
		m_context->ProcessMouseButtonUp(button, mods);
}

void RmlUIInputBridge::ProcessMouseScroll(Float32 dx, Float32 dy)
{
	if (!m_context) return;
	m_context->ProcessMouseWheel({ dx, dy }, BuildModifiers());
}

void RmlUIInputBridge::ProcessKey(Int32 glfw_key, bool pressed)
{
	if (!m_context) return;
	auto rml_key = static_cast<Rml::Input::KeyIdentifier>(MapKey(glfw_key));
	int mods = BuildModifiers();
	if (pressed)
		m_context->ProcessKeyDown(rml_key, mods);
	else
		m_context->ProcessKeyUp(rml_key, mods);
}

void RmlUIInputBridge::ProcessChar(UInt32 codepoint)
{
	if (!m_context) return;
	m_context->ProcessTextInput(static_cast<Rml::Character>(codepoint));
}

void RmlUIInputBridge::ProcessMouseLeave()
{
	if (!m_context) return;
	m_context->ProcessMouseLeave();
}

bool RmlUIInputBridge::IsMouseInteracting() CONST
{
	if (!m_context) return false;
	return m_context->IsMouseInteracting();
}

void RmlUIInputBridge::SetInputPriority(bool game_first)
{
	m_game_first = game_first;
}

Int32 RmlUIInputBridge::BuildModifiers() CONST
{
	int mods = 0;
	// Check immediate mode: glfwGetKey returns current key state
	auto* glfw_ctx = glfwGetCurrentContext();
	if (!glfw_ctx) return 0;

	if (glfwGetKey(glfw_ctx, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(glfw_ctx, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		mods |= Rml::Input::KM_CTRL;
	if (glfwGetKey(glfw_ctx, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(glfw_ctx, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
		mods |= Rml::Input::KM_SHIFT;
	if (glfwGetKey(glfw_ctx, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
		glfwGetKey(glfw_ctx, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
		mods |= Rml::Input::KM_ALT;
	if (glfwGetKey(glfw_ctx, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
		mods |= Rml::Input::KM_CAPSLOCK;
	if (glfwGetKey(glfw_ctx, GLFW_KEY_NUM_LOCK) == GLFW_PRESS)
		mods |= Rml::Input::KM_NUMLOCK;

	return mods;
}

Int32 RmlUIInputBridge::MapKey(Int32 glfw_key)
{
	// GLFW → RmlUI key identifier mapping
	// Covers the most common keys used in game UI.
	switch (glfw_key)
	{
	case GLFW_KEY_SPACE:         return static_cast<Int32>(Rml::Input::KI_SPACE);
	case GLFW_KEY_0:             return static_cast<Int32>(Rml::Input::KI_0);
	case GLFW_KEY_1:             return static_cast<Int32>(Rml::Input::KI_1);
	case GLFW_KEY_2:             return static_cast<Int32>(Rml::Input::KI_2);
	case GLFW_KEY_3:             return static_cast<Int32>(Rml::Input::KI_3);
	case GLFW_KEY_4:             return static_cast<Int32>(Rml::Input::KI_4);
	case GLFW_KEY_5:             return static_cast<Int32>(Rml::Input::KI_5);
	case GLFW_KEY_6:             return static_cast<Int32>(Rml::Input::KI_6);
	case GLFW_KEY_7:             return static_cast<Int32>(Rml::Input::KI_7);
	case GLFW_KEY_8:             return static_cast<Int32>(Rml::Input::KI_8);
	case GLFW_KEY_9:             return static_cast<Int32>(Rml::Input::KI_9);
	case GLFW_KEY_A:             return static_cast<Int32>(Rml::Input::KI_A);
	case GLFW_KEY_B:             return static_cast<Int32>(Rml::Input::KI_B);
	case GLFW_KEY_C:             return static_cast<Int32>(Rml::Input::KI_C);
	case GLFW_KEY_D:             return static_cast<Int32>(Rml::Input::KI_D);
	case GLFW_KEY_E:             return static_cast<Int32>(Rml::Input::KI_E);
	case GLFW_KEY_F:             return static_cast<Int32>(Rml::Input::KI_F);
	case GLFW_KEY_G:             return static_cast<Int32>(Rml::Input::KI_G);
	case GLFW_KEY_H:             return static_cast<Int32>(Rml::Input::KI_H);
	case GLFW_KEY_I:             return static_cast<Int32>(Rml::Input::KI_I);
	case GLFW_KEY_J:             return static_cast<Int32>(Rml::Input::KI_J);
	case GLFW_KEY_K:             return static_cast<Int32>(Rml::Input::KI_K);
	case GLFW_KEY_L:             return static_cast<Int32>(Rml::Input::KI_L);
	case GLFW_KEY_M:             return static_cast<Int32>(Rml::Input::KI_M);
	case GLFW_KEY_N:             return static_cast<Int32>(Rml::Input::KI_N);
	case GLFW_KEY_O:             return static_cast<Int32>(Rml::Input::KI_O);
	case GLFW_KEY_P:             return static_cast<Int32>(Rml::Input::KI_P);
	case GLFW_KEY_Q:             return static_cast<Int32>(Rml::Input::KI_Q);
	case GLFW_KEY_R:             return static_cast<Int32>(Rml::Input::KI_R);
	case GLFW_KEY_S:             return static_cast<Int32>(Rml::Input::KI_S);
	case GLFW_KEY_T:             return static_cast<Int32>(Rml::Input::KI_T);
	case GLFW_KEY_U:             return static_cast<Int32>(Rml::Input::KI_U);
	case GLFW_KEY_V:             return static_cast<Int32>(Rml::Input::KI_V);
	case GLFW_KEY_W:             return static_cast<Int32>(Rml::Input::KI_W);
	case GLFW_KEY_X:             return static_cast<Int32>(Rml::Input::KI_X);
	case GLFW_KEY_Y:             return static_cast<Int32>(Rml::Input::KI_Y);
	case GLFW_KEY_Z:             return static_cast<Int32>(Rml::Input::KI_Z);
	case GLFW_KEY_ESCAPE:        return static_cast<Int32>(Rml::Input::KI_ESCAPE);
	case GLFW_KEY_TAB:           return static_cast<Int32>(Rml::Input::KI_TAB);
	case GLFW_KEY_BACKSPACE:     return static_cast<Int32>(Rml::Input::KI_BACK);
	case GLFW_KEY_ENTER:         return static_cast<Int32>(Rml::Input::KI_RETURN);
	case GLFW_KEY_LEFT:          return static_cast<Int32>(Rml::Input::KI_LEFT);
	case GLFW_KEY_RIGHT:         return static_cast<Int32>(Rml::Input::KI_RIGHT);
	case GLFW_KEY_UP:            return static_cast<Int32>(Rml::Input::KI_UP);
	case GLFW_KEY_DOWN:          return static_cast<Int32>(Rml::Input::KI_DOWN);
	case GLFW_KEY_INSERT:        return static_cast<Int32>(Rml::Input::KI_INSERT);
	case GLFW_KEY_DELETE:        return static_cast<Int32>(Rml::Input::KI_DELETE);
	case GLFW_KEY_HOME:          return static_cast<Int32>(Rml::Input::KI_HOME);
	case GLFW_KEY_END:           return static_cast<Int32>(Rml::Input::KI_END);
	case GLFW_KEY_PAGE_UP:       return static_cast<Int32>(Rml::Input::KI_PRIOR);
	case GLFW_KEY_PAGE_DOWN:     return static_cast<Int32>(Rml::Input::KI_NEXT);
	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT:   return static_cast<Int32>(Rml::Input::KI_LSHIFT);
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL: return static_cast<Int32>(Rml::Input::KI_LCONTROL);
	case GLFW_KEY_LEFT_ALT:
	case GLFW_KEY_RIGHT_ALT:     return static_cast<Int32>(Rml::Input::KI_LMENU);
	case GLFW_KEY_F1:            return static_cast<Int32>(Rml::Input::KI_F1);
	case GLFW_KEY_F2:            return static_cast<Int32>(Rml::Input::KI_F2);
	case GLFW_KEY_F3:            return static_cast<Int32>(Rml::Input::KI_F3);
	case GLFW_KEY_F4:            return static_cast<Int32>(Rml::Input::KI_F4);
	case GLFW_KEY_F5:            return static_cast<Int32>(Rml::Input::KI_F5);
	case GLFW_KEY_F6:            return static_cast<Int32>(Rml::Input::KI_F6);
	case GLFW_KEY_F7:            return static_cast<Int32>(Rml::Input::KI_F7);
	case GLFW_KEY_F8:            return static_cast<Int32>(Rml::Input::KI_F8);
	case GLFW_KEY_F9:            return static_cast<Int32>(Rml::Input::KI_F9);
	case GLFW_KEY_F10:           return static_cast<Int32>(Rml::Input::KI_F10);
	case GLFW_KEY_F11:           return static_cast<Int32>(Rml::Input::KI_F11);
	case GLFW_KEY_F12:           return static_cast<Int32>(Rml::Input::KI_F12);
	case GLFW_KEY_PAUSE:         return static_cast<Int32>(Rml::Input::KI_PAUSE);
	case GLFW_KEY_KP_0:          return static_cast<Int32>(Rml::Input::KI_NUMPAD0);
	case GLFW_KEY_KP_1:          return static_cast<Int32>(Rml::Input::KI_NUMPAD1);
	case GLFW_KEY_KP_2:          return static_cast<Int32>(Rml::Input::KI_NUMPAD2);
	case GLFW_KEY_KP_3:          return static_cast<Int32>(Rml::Input::KI_NUMPAD3);
	case GLFW_KEY_KP_4:          return static_cast<Int32>(Rml::Input::KI_NUMPAD4);
	case GLFW_KEY_KP_5:          return static_cast<Int32>(Rml::Input::KI_NUMPAD5);
	case GLFW_KEY_KP_6:          return static_cast<Int32>(Rml::Input::KI_NUMPAD6);
	case GLFW_KEY_KP_7:          return static_cast<Int32>(Rml::Input::KI_NUMPAD7);
	case GLFW_KEY_KP_8:          return static_cast<Int32>(Rml::Input::KI_NUMPAD8);
	case GLFW_KEY_KP_9:          return static_cast<Int32>(Rml::Input::KI_NUMPAD9);
	case GLFW_KEY_KP_ENTER:      return static_cast<Int32>(Rml::Input::KI_NUMPADENTER);
	case GLFW_KEY_KP_MULTIPLY:   return static_cast<Int32>(Rml::Input::KI_MULTIPLY);
	case GLFW_KEY_KP_ADD:        return static_cast<Int32>(Rml::Input::KI_ADD);
	case GLFW_KEY_KP_SUBTRACT:   return static_cast<Int32>(Rml::Input::KI_SUBTRACT);
	case GLFW_KEY_KP_DECIMAL:    return static_cast<Int32>(Rml::Input::KI_DECIMAL);
	case GLFW_KEY_KP_DIVIDE:     return static_cast<Int32>(Rml::Input::KI_DIVIDE);
	default:                     return static_cast<Int32>(Rml::Input::KI_UNKNOWN);
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
