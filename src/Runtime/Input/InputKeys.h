#pragma once
#ifndef _INPUT_KEYS_
#define _INPUT_KEYS_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Input)

// Lightweight platform-neutral key identifier (like UE's FKey).
MYRENDERER_BEGIN_STRUCT(Key)
public:
	String name;

	Key() MYDEFAULT;
	explicit Key(const String& in_name) : name(in_name) {}

	Bool operator==(CONST Key& o) CONST { return name == o.name; }
	Bool operator!=(CONST Key& o) CONST { return name != o.name; }
	Bool operator<(CONST Key& o) CONST { return name < o.name; }

	Bool IsKeyboard() CONST;
	Bool IsMouse()    CONST;
	Bool IsGamepad()  CONST;
	Bool IsTouch()    CONST;
MYRENDERER_END_STRUCT

// Pre-defined key constants (like UE's EKeys).
namespace EKey
{
	// Keyboard
	extern CONST Key A, B, C, D, E, F, G, H, I, J, K, L, M;
	extern CONST Key N, O, P, Q, R, S, T, U, V, W, X, Y, Z;
	extern CONST Key Space, Enter, Escape, Tab, Backspace, Delete;
	extern CONST Key LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt;
	extern CONST Key Up, Down, Left, Right;
	extern CONST Key F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12;
	extern CONST Key K0, K1, K2, K3, K4, K5, K6, K7, K8, K9;

	// Mouse
	extern CONST Key LeftMouse, RightMouse, MiddleMouse;
	extern CONST Key MouseX, MouseY, MouseWheel;

	// Touch (virtual keys)
	extern CONST Key Touch1, Touch2, Touch3;
}

// Platform-specific key code mapping (implemented per-platform).
// Maps native key codes (GLFW_KEY_*, AKEYCODE_*, etc.) to EKey.
namespace PlatformKeyMap
{
	// Called once at init to register platform-specific mappings.
	void RegisterDesktop();  // GLFW key codes
	void RegisterAndroid();  // Android AKEYCODE_* key codes

	// Lookup: platform_code → Key, Key → platform_code
	CONST Key& FromCode(Int code);
	Int        ToCode(CONST Key& key);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
