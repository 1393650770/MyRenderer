#include "CameraController.h"
#include "Platform/PlatformWindow.h"
#include "Input/InputSystem.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

OrbitCameraController::~OrbitCameraController() MYDEFAULT;

void OrbitCameraController::Attach(PlatformWindow* in_window)
{
	window = in_window;
	if (window)
	{
		Float64 x, y;
		window->GetCursorPos(x, y);
		last_cursor_x = x;
		last_cursor_y = y;
	}
}

Float32 OrbitCameraController::ConsumeScrollDelta()
{
	return MXRender::Input::InputSystem::Get().GetScrollDelta();
}

void OrbitCameraController::Update(Float32 in_dt, Render::SceneView& out_view)
{
	if (window == nullptr)
		return;
	Int width = 0, height = 0;
	window->GetFramebufferSize(width, height);

	auto& input = MXRender::Input::InputSystem::Get();
	Float32 cx = 0, cy = 0;
	input.GetMousePos(cx, cy);
	Float32 delta_x = cx - (Float32)last_cursor_x;
	Float32 delta_y = cy - (Float32)last_cursor_y;

	Bool lmb_down = input.IsMouseDown((Int)MouseButton::Left);
	if (lmb_down && rotating)
	{
		yaw += delta_x * rotate_speed;
		pitch = glm::clamp(pitch + delta_y * rotate_speed, min_pitch, max_pitch);
	}
	rotating = lmb_down;

	Bool mmb_down = input.IsMouseDown((Int)MouseButton::Middle);
	if (enable_pan && mmb_down && panning && height > 0)
	{
		glm::vec3 forward = -glm::vec3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw));
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));
		Float32 pan_scale = distance / (Float32)height;
		target += (-delta_x * right + delta_y * up) * pan_scale;
	}
	panning = mmb_down;

	last_cursor_x = (Float64)cx;
	last_cursor_y = (Float64)cy;

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
