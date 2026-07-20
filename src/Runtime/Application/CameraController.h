#pragma once
#ifndef _CAMERACONTROLLER_
#define _CAMERACONTROLLER_
#include "Core/ConstDefine.h"
#include "Platform/PlatformWindow.h"
#include "Render/View/SceneView.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

// Interactive orbit camera (yaw/pitch/distance around a target point).
// Input mapping (polled from PlatformWindow each Update):
//   LMB drag   - rotate (yaw/pitch)
//   scroll     - zoom (exponential distance scaling)
//   MMB drag   - pan the target in the view plane (if enable_pan)
MYRENDERER_BEGIN_CLASS(OrbitCameraController)
#pragma region METHOD
public:
	OrbitCameraController() MYDEFAULT;
	~OrbitCameraController();

	void METHOD(Attach)(PlatformWindow* in_window);
	void METHOD(Update)(Float32 in_dt, Render::SceneView& out_view);
protected:
	static void METHOD(OnScroll)(Float64 in_offset_x, Float64 in_offset_y);
	Float32 METHOD(ConsumeScrollDelta)();
private:
#pragma endregion

#pragma region MEMBER
public:
	Float32 yaw{ 0.7f };
	Float32 pitch{ 0.25f };
	Float32 distance{ 10.0f };
	glm::vec3 target{ 0.0f, 0.0f, 0.0f };
	Float32 rotate_speed{ 0.005f };
	Float32 zoom_step{ 0.9f };
	Float32 min_distance{ 0.5f };
	Float32 max_distance{ 500.0f };
	Float32 min_pitch{ -1.45f };
	Float32 max_pitch{ 1.45f };
	Bool enable_pan{ true };
protected:
	PlatformWindow* window{ nullptr };
	Bool rotating{ false };
	Bool panning{ false };
	Float64 last_cursor_x{ 0.0 };
	Float64 last_cursor_y{ 0.0 };
	Float32 scroll_accum{ 0.0f };
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
