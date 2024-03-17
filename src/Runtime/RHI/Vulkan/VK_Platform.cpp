#include "VK_Platform.h"
#ifndef GLFW_INCLUDE_VULKAN
#define  GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
void VK_Platform::CreateSurface(void* window_handle, VkInstance instance, VkSurfaceKHR* out_suface)
{
    GLFWwindow* window = STATIC_CAST(window_handle,GLFWwindow);
    CHECK_WITH_LOG(glfwCreateWindowSurface(instance, window, nullptr, out_suface) != VK_SUCCESS,"RHI Error: Create Window Surface Error");
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
