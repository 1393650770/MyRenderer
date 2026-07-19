#pragma once
#ifndef _SCENEVIEW_
#define _SCENEVIEW_
// Unified Vulkan glm conventions for the whole engine:
// depth range 0..1 (GLM_FORCE_DEPTH_ZERO_TO_ONE) + Y-flip baked into the
// projection matrix (proj[1][1] *= -1) inside UpdateMatrices().
// NOTE: the glm force-macro takes effect on the FIRST glm include of a TU -
// include SceneView.h before any other glm include (or define it yourself).
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Camera view/projection matrix set, modeled after UE's FSceneView/FViewMatrices
// (accessor naming: GetViewMatrix / GetProjectionMatrix / GetViewProjectionMatrix /
// GetInvViewProjectionMatrix / DeprojectScreenToWorld). Pure math - no RHI,
// no input dependency; drive it from a camera controller or set it directly.
MYRENDERER_BEGIN_CLASS(SceneView)
#pragma region METHOD
public:
	SceneView() MYDEFAULT;
	~SceneView() MYDEFAULT;

	void METHOD(SetPerspective)(Float32 in_fov_y_radians, Float32 in_near_plane, Float32 in_far_plane);
	void METHOD(SetViewport)(UInt32 in_width, UInt32 in_height);
	void METHOD(SetViewLookAt)(CONST glm::vec3& in_eye, CONST glm::vec3& in_target, CONST glm::vec3& in_up);
	// Rebuild view/projection/view_projection/inv_view_projection from the
	// current parameters. Projection = glm::perspective (0..1 depth) with the
	// Vulkan Y-flip applied.
	void METHOD(UpdateMatrices)();

	CONST glm::mat4& METHOD(GetViewMatrix)() CONST { return view; }
	CONST glm::mat4& METHOD(GetProjectionMatrix)() CONST { return projection; }
	CONST glm::mat4& METHOD(GetViewProjectionMatrix)() CONST { return view_projection; }
	CONST glm::mat4& METHOD(GetInvViewProjectionMatrix)() CONST { return inv_view_projection; }
	CONST glm::vec3& METHOD(GetViewLocation)() CONST { return view_location; }
	CONST glm::vec3& METHOD(GetViewTarget)() CONST { return view_target; }
	UInt32 METHOD(GetViewportWidth)() CONST { return viewport_width; }
	UInt32 METHOD(GetViewportHeight)() CONST { return viewport_height; }
	Float32 METHOD(GetAspectRatio)() CONST;

	// Screen pixel coordinates -> world-space ray (near-plane origin +
	// normalized direction), via inverse view-projection deprojection.
	void METHOD(DeprojectScreenToWorld)(Float32 in_screen_x, Float32 in_screen_y, glm::vec3& out_ray_origin, glm::vec3& out_ray_direction) CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	glm::vec3 view_location{ 0.0f, 5.0f, 10.0f };
	glm::vec3 view_target{ 0.0f, 0.0f, 0.0f };
	glm::vec3 view_up{ 0.0f, 1.0f, 0.0f };
	Float32 fov_y{ glm::radians(45.0f) };
	Float32 near_plane{ 0.1f };
	Float32 far_plane{ 1000.0f };
	UInt32 viewport_width{ 1280 };
	UInt32 viewport_height{ 960 };
	glm::mat4 view{ 1.0f };
	glm::mat4 projection{ 1.0f };
	glm::mat4 view_projection{ 1.0f };
	glm::mat4 inv_view_projection{ 1.0f };
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
