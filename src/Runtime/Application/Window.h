#pragma once
#ifndef _WINDOW_
#define _WINDOW_
#include "Core/ConstDefine.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include <functional>
#include <vector>
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

class MyRender;

MYRENDERER_BEGIN_CLASS(Window)
#pragma region METHOD
	public:
		Window();
		VIRTUAL ~Window();
		void METHOD(InitWindow)();
		void METHOD(Run)(MyRender* render);
		GLFWwindow* METHOD(GetWindow)() CONST;

	protected:
	public:
#pragma endregion

#pragma region MEMBER
	public:
	protected:
		GLFWwindow* window = nullptr;
		Float32 deltaTime = 0.0f;
		Float32 lastFrame = 0.0f;
		UInt32 width = 1600, height = 1000;
		Bool is_full_screen = false;
		MXRender::RHI::Viewport* viewport_rhi =nullptr;
	private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //_WINDOW_

