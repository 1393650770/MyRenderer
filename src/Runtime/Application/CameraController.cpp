#include "CameraController.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

// Single-window engine: static owner routes the scroll callback.
static OrbitCameraController* s_scroll_owner = nullptr;

void OrbitCameraController::OnScroll(Float64 in_offset_x, Float64 in_offset_y)
{
	if (s_scroll_owner)
		s_scroll_owner->scroll_accum += (Float32)in_offset_y;
}

void OrbitCameraController::Attach(PlatformWindow* in_window)
{
	window = in_window;
	s_scroll_owner = this;
	if (window)
	{
		window->SetScrollCallback(&OrbitCameraController::OnScroll);
		window->GetCursorPos(last_cursor_x, last_cursor_y);
	}
}

OrbitCameraController::~OrbitCameraController()
{
	if (s_scroll_owner == this)
		s_scroll_owner = nullptr;
}

Float32 OrbitCameraController::ConsumeScrollDelta()
{
	Float32 delta = scroll_accum;
	scroll_accum = 0.0f;
	return delta;
}

void OrbitCameraController::Update(Float32 in_dt, Render::SceneView& out_view)
{
	if (window == nullptr)
		return;
	Int width = 0, height = 0;
	window->GetFramebufferSize(width, height);

	Float64 cursor_x = 0.0, cursor_y = 0.0;
	window->GetCursorPos(cursor_x, cursor_y);
	Float32 delta_x = (Float32)(cursor_x - last_cursor_x);
	Float32 delta_y = (Float32)(cursor_y - last_cursor_y);

	Bool lmb_down = window->GetMouseButton(MouseButton::Left);
	if (lmb_down && rotating)
	{
		yaw += delta_x * rotate_speed;
		pitch = glm::clamp(pitch + delta_y * rotate_speed, min_pitch, max_pitch);
	}
	rotating = lmb_down;

	Bool mmb_down = window->GetMouseButton(MouseButton::Middle);
	if (enable_pan && mmb_down && panning && height > 0)
	{
		glm::vec3 forward = -glm::vec3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw));
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));
		Float32 pan_scale = distance / (Float32)height;
		target += (-delta_x * right + delta_y * up) * pan_scale;
	}
	panning = mmb_down;

	last_cursor_x = cursor_x;
	last_cursor_y = cursor_y;

	Float32 scroll = ConsumeScrollDelta();
	if (scroll != 0.0f)
		distance = glm::clamp(distance * powf(zoom_step, scroll), min_distance, max_distance);

	if (width <= 0 || height <= 0)
		return;

	glm::vec3 eye = target + distance * glm::vec3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw));
	out_view.SetViewport((UInt32)width, (UInt32)height);
	out_view.SetViewLookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));
	out_view.UpdateMatrices();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
