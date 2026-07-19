#include "SceneView.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

void SceneView::SetPerspective(Float32 in_fov_y_radians, Float32 in_near_plane, Float32 in_far_plane)
{
	fov_y = in_fov_y_radians;
	near_plane = in_near_plane;
	far_plane = in_far_plane;
}

void SceneView::SetViewport(UInt32 in_width, UInt32 in_height)
{
	viewport_width = in_width;
	viewport_height = in_height;
}

void SceneView::SetViewLookAt(CONST glm::vec3& in_eye, CONST glm::vec3& in_target, CONST glm::vec3& in_up)
{
	view_location = in_eye;
	view_target = in_target;
	view_up = in_up;
}

Float32 SceneView::GetAspectRatio() CONST
{
	if (viewport_height == 0)
		return 1.0f;
	return (Float32)viewport_width / (Float32)viewport_height;
}

void SceneView::UpdateMatrices()
{
	view = glm::lookAt(view_location, view_target, view_up);
	projection = glm::perspective(fov_y, GetAspectRatio(), near_plane, far_plane);
	// Vulkan clip space: Y points down. Bake the flip here once so every
	// consumer (samples, editor viewport) shares the same convention.
	projection[1][1] *= -1.0f;
	view_projection = projection * view;
	inv_view_projection = glm::inverse(view_projection);
}

void SceneView::DeprojectScreenToWorld(Float32 in_screen_x, Float32 in_screen_y, glm::vec3& out_ray_origin, glm::vec3& out_ray_direction) CONST
{
	// pixel -> Vulkan NDC. With the Y-flip baked into the projection, NDC Y
	// points down (top of screen = -1), so screen_y maps directly:
	Float32 ndc_x = 2.0f * in_screen_x / (Float32)viewport_width - 1.0f;
	Float32 ndc_y = 2.0f * in_screen_y / (Float32)viewport_height - 1.0f;
	// 0..1 depth range: near plane z = 0, far plane z = 1
	glm::vec4 near_h = inv_view_projection * glm::vec4(ndc_x, ndc_y, 0.0f, 1.0f);
	glm::vec4 far_h = inv_view_projection * glm::vec4(ndc_x, ndc_y, 1.0f, 1.0f);
	glm::vec3 near_p = glm::vec3(near_h) / near_h.w;
	glm::vec3 far_p = glm::vec3(far_h) / far_h.w;
	out_ray_origin = near_p;
	out_ray_direction = glm::normalize(far_p - near_p);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
