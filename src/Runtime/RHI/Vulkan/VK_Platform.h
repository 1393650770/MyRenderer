#pragma once

#ifndef _VK_VIEWPORT_
#define _VK_VIEWPORT_
#include "../Viewport.h"
#include "vulkan/vulkan_core.h"



MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


MYRENDERER_BEGIN_CLASS(VK_Platform)
#pragma region METHOD
public:
	static void METHOD(CreateSurface)(void* window_handle,VkInstance instance,VkSurfaceKHR* out_suface);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	
private:

#pragma endregion


MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
