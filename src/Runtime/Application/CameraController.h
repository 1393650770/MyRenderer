#pragma once
#ifndef _CAMERACONTROLLER_
#define _CAMERACONTROLLER_
// NOTE: deliberately does NOT include Window.h / GLFW here - Window.h pulls in
// windows.h (min/max macro pollution). Only the .cpp talks to GLFW.
#include "Core/ConstDefine.h"
#include "Render/View/SceneView.h"

struct GLFWwindow;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

// Interactive orbit camera (yaw/pitch/distance around a target point).
// Input mapping (polled from GLFW each Update, no input system needed):
//   LMB drag   - rotate (yaw/pitch)
//   scroll     - zoom (exponential distance scaling)
//   MMB drag   - pan the target in the view plane (if enable_pan)
// Feel/defaults collected from the Ocean sample's hand-tuned camera.
MYRENDERER_BEGIN_CLASS(OrbitCameraController)
#pragma region METHOD
public:
	OrbitCameraController() MYDEFAULT;
	~OrbitCameraController();

	// Bind a window and register the scroll callback. GLFW allows only ONE
	// scroll callback per window - Attach overwrites any existing one (samples
	// that call glfwSetScrollCallback manually must not Attach, and vice versa).
	void METHOD(Attach)(GLFWwindow* in_window);
	// Poll input, advance the orbit parameters, then write the resulting
	// camera into out_view (SetViewport + SetViewLookAt + UpdateMatrices).
	// Call once per frame from OnUpdate (Logic thread).
	void METHOD(Update)(Float32 in_dt, Render::SceneView& out_view);
protected:
	static void METHOD(OnScroll)(GLFWwindow* in_window, Float64 in_offset_x, Float64 in_offset_y);
	Float32 METHOD(ConsumeScrollDelta)();
private:

#pragma endregion

#pragma region MEMBER
public:
	// Orbit parameters - tweak freely before/between Updates.
	Float32 yaw{ 0.7f };
	Float32 pitch{ 0.25f };
	Float32 distance{ 10.0f };
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };
	Float32 rotate_speed{ 0.005f };
	Float32 zoom_step{ 0.9f };            // distance *= pow(zoom_step, scroll)
	Float32 min_distance{ 0.5f };
	Float32 max_distance{ 500.0f };
	Float32 min_pitch{ -1.45f };
	Float32 max_pitch{ 1.45f };
	Bool enable_pan{ true };
protected:
	GLFWwindow* window{ nullptr };
	Bool rotating{ false };
	Bool panning{ false };
	Float64 last_cursor_x{ 0.0 };
	Float64 last_cursor_y{ 0.0 };
	Float32 scroll_accum{ 0.0f };         // written by OnScroll, consumed by Update
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
