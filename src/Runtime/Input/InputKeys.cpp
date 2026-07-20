#include "InputKeys.h"
#include <unordered_map>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Input)

// Key category helpers
Bool Key::IsKeyboard() CONST { return name[0] != 'M' && name != "Left" + String(""); /* rough heuristic */ }
Bool Key::IsMouse() CONST
{
	return name == EKey::LeftMouse.name || name == EKey::RightMouse.name
		|| name == EKey::MiddleMouse.name || name == EKey::MouseX.name
		|| name == EKey::MouseY.name || name == EKey::MouseWheel.name;
}
Bool Key::IsGamepad() CONST { return name.find("Gamepad_") == 0; }
Bool Key::IsTouch() CONST { return name.find("Touch") == 0; }

// ===== EKey constants =====
#define DEF_KEY(name) CONST Key name(#name)

namespace EKey
{
	DEF_KEY(A); DEF_KEY(B); DEF_KEY(C); DEF_KEY(D); DEF_KEY(E);
	DEF_KEY(F); DEF_KEY(G); DEF_KEY(H); DEF_KEY(I); DEF_KEY(J);
	DEF_KEY(K); DEF_KEY(L); DEF_KEY(M); DEF_KEY(N); DEF_KEY(O);
	DEF_KEY(P); DEF_KEY(Q); DEF_KEY(R); DEF_KEY(S); DEF_KEY(T);
	DEF_KEY(U); DEF_KEY(V); DEF_KEY(W); DEF_KEY(X); DEF_KEY(Y); DEF_KEY(Z);
	DEF_KEY(Space); DEF_KEY(Enter); DEF_KEY(Escape); DEF_KEY(Tab);
	DEF_KEY(Backspace); DEF_KEY(Delete);
	DEF_KEY(LeftShift); DEF_KEY(RightShift); DEF_KEY(LeftCtrl);
	DEF_KEY(RightCtrl); DEF_KEY(LeftAlt); DEF_KEY(RightAlt);
	DEF_KEY(Up); DEF_KEY(Down); DEF_KEY(Left); DEF_KEY(Right);
	DEF_KEY(F1); DEF_KEY(F2); DEF_KEY(F3); DEF_KEY(F4); DEF_KEY(F5); DEF_KEY(F6);
	DEF_KEY(F7); DEF_KEY(F8); DEF_KEY(F9); DEF_KEY(F10); DEF_KEY(F11); DEF_KEY(F12);
	DEF_KEY(K0); DEF_KEY(K1); DEF_KEY(K2); DEF_KEY(K3); DEF_KEY(K4);
	DEF_KEY(K5); DEF_KEY(K6); DEF_KEY(K7); DEF_KEY(K8); DEF_KEY(K9);

	DEF_KEY(LeftMouse); DEF_KEY(RightMouse); DEF_KEY(MiddleMouse);
	DEF_KEY(MouseX); DEF_KEY(MouseY); DEF_KEY(MouseWheel);
	DEF_KEY(Touch1); DEF_KEY(Touch2); DEF_KEY(Touch3);
}
#undef DEF_KEY

// ===== Platform key code registry =====
static std::unordered_map<Int, Key> s_code_to_key;
static std::unordered_map<String, Int> s_key_to_code;

static void Register(Int code, CONST Key& key)
{
	s_code_to_key[code] = key;
	s_key_to_code[key.name] = code;
}

void PlatformKeyMap::RegisterDesktop()
{
	// Map GLFW key codes to EKey
	Register(65, EKey::A);  Register(66, EKey::B);  Register(67, EKey::C);
	Register(68, EKey::D);  Register(69, EKey::E);  Register(70, EKey::F);
	Register(71, EKey::G);  Register(72, EKey::H);  Register(73, EKey::I);
	Register(74, EKey::J);  Register(75, EKey::K);  Register(76, EKey::L);
	Register(77, EKey::M);  Register(78, EKey::N);  Register(79, EKey::O);
	Register(80, EKey::P);  Register(81, EKey::Q);  Register(82, EKey::R);
	Register(83, EKey::S);  Register(84, EKey::T);  Register(85, EKey::U);
	Register(86, EKey::V);  Register(87, EKey::W);  Register(88, EKey::X);
	Register(89, EKey::Y);  Register(90, EKey::Z);
	Register(32,  EKey::Space);   Register(257, EKey::Enter);
	Register(256, EKey::Escape);  Register(258, EKey::Tab);
	Register(259, EKey::Backspace); Register(261, EKey::Delete);
	Register(340, EKey::LeftShift); Register(344, EKey::RightShift);
	Register(341, EKey::LeftCtrl);  Register(345, EKey::RightCtrl);
	Register(342, EKey::LeftAlt);   Register(346, EKey::RightAlt);
	Register(265, EKey::Up);    Register(264, EKey::Down);
	Register(263, EKey::Left);  Register(262, EKey::Right);
	Register(290, EKey::F1);  Register(291, EKey::F2);  Register(292, EKey::F3);
	Register(293, EKey::F4);  Register(294, EKey::F5);  Register(295, EKey::F6);
	Register(296, EKey::F7);  Register(297, EKey::F8);  Register(298, EKey::F9);
	Register(299, EKey::F10); Register(300, EKey::F11); Register(301, EKey::F12);
	Register(48, EKey::K0); Register(49, EKey::K1); Register(50, EKey::K2);
	Register(51, EKey::K3); Register(52, EKey::K4); Register(53, EKey::K5);
	Register(54, EKey::K6); Register(55, EKey::K7); Register(56, EKey::K8); Register(57, EKey::K9);
}

void PlatformKeyMap::RegisterAndroid()
{
	// Map Android AKEYCODE_* to EKey. GLFW codes on desktop already work.
	// Android uses different key codes; for now use the same mapping
	// as most letter/number keys match. In production, map AKEYCODE_* values.
	RegisterDesktop(); // fallback: same codes for common keys
}

CONST Key& PlatformKeyMap::FromCode(Int code)
{
	static Key s_none;
	auto it = s_code_to_key.find(code);
	return it != s_code_to_key.end() ? it->second : s_none;
}

Int PlatformKeyMap::ToCode(CONST Key& key)
{
	auto it = s_key_to_code.find(key.name);
	return it != s_key_to_code.end() ? it->second : -1;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
