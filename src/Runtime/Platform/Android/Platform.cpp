#if PLATFORM_ANDROID
#include "Platform/Platform.h"
#include "RHI/Vulkan/VK_RenderRHI.h"

// Symbol referenced by VK_Device.cpp / VK_ResourcePool.cpp
namespace MXRender { namespace RHI { namespace Vulkan {
__attribute__((visibility("default"))) VkDevice g_debug_name_device = VK_NULL_HANDLE;
} } }

MXRender::RHI::RenderRHI* PlatformCreateDynamicRHI()
{
	MXRender::RHI::Vulkan::VulkanRHI* pRHI = new MXRender::RHI::Vulkan::VulkanRHI();
	MXRender::RHI::Vulkan::VulkanRenderFactory factory;
	factory.enable_render_debug = false;
	factory.threading_mode = EThreadingMode::Single;
	pRHI->Init(&factory);

	return pRHI;
}
#endif
