#include "VK_Platform.h"
#if PLATFORM_ANDROID
#include <vulkan/vulkan_android.h>
#include <android/native_window.h>
#else
#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
void VK_Platform::CreateSurface(void* window_handle, VkInstance instance, VkSurfaceKHR* out_suface)
{
#if PLATFORM_ANDROID
    ANativeWindow* window = STATIC_CAST(window_handle, ANativeWindow);
    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = window;
    CHECK_WITH_LOG(vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, out_suface) != VK_SUCCESS, "RHI Error: Create Android Window Surface Error");
#else
    GLFWwindow* window = STATIC_CAST(window_handle, GLFWwindow);
    CHECK_WITH_LOG(glfwCreateWindowSurface(instance, window, nullptr, out_suface) != VK_SUCCESS, "RHI Error: Create Window Surface Error");
#endif
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
