#include "CameraController.h"
#include <GLFW/glfw3.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

// Single-window engine: one static owner is enough to route the C callback
// back to the controller instance (extend to a map if multi-window arrives).
static OrbitCameraController* s_scroll_owner = nullptr;

void OrbitCameraController::Attach(GLFWwindow* in_window)
{
	window = in_window;
	s_scroll_owner = this;
	glfwSetScrollCallback(window, &OrbitCameraController::OnScroll);
	if (window)
		glfwGetCursorPos(window, &last_cursor_x, &last_cursor_y);
}

OrbitCameraController::~OrbitCameraController()
{
	if (s_scroll_owner == this)
		s_scroll_owner = nullptr;
}

void OrbitCameraController::OnScroll(GLFWwindow* in_window, Float64 in_offset_x, Float64 in_offset_y)
{
	if (s_scroll_owner != nullptr && s_scroll_owner->window == in_window)
		s_scroll_owner->scroll_accum += (Float32)in_offset_y;
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
	glfwGetWindowSize(window, &width, &height);

	Float64 cursor_x = 0.0, cursor_y = 0.0;
	glfwGetCursorPos(window, &cursor_x, &cursor_y);
	Float32 delta_x = (Float32)(cursor_x - last_cursor_x);
	Float32 delta_y = (Float32)(cursor_y - last_cursor_y);

	// LMB drag: rotate. Cursor deltas are already frame-rate independent.
	Bool lmb_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (lmb_down && rotating)
	{
		yaw += delta_x * rotate_speed;
		pitch = glm::clamp(pitch + delta_y * rotate_speed, min_pitch, max_pitch);
	}
	rotating = lmb_down;

	// MMB drag: pan the target along the camera right/up axes, speed
	// proportional to distance so the scene follows the cursor.
	Bool mmb_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
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

	// Scroll: exponential zoom (Ocean feel).
	Float32 scroll = ConsumeScrollDelta();
	if (scroll != 0.0f)
		distance = glm::clamp(distance * powf(zoom_step, scroll), min_distance, max_distance);

	if (width <= 0 || height <= 0)
		return; // minimized - keep last matrices

	glm::vec3 eye = target + distance * glm::vec3(cosf(pitch) * sinf(yaw), sinf(pitch), cosf(pitch) * cosf(yaw));
	out_view.SetViewport((UInt32)width, (UInt32)height);
	out_view.SetViewLookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));
	out_view.UpdateMatrices();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
