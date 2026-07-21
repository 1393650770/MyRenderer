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
	factory.validation_level = 0;        // Set to 1 or 2 to enable Vulkan validation layers
	factory.enable_debug_callback = false;
	factory.validation_optional = true;  //  UE behaviour: warn but don't crash if layers unavailable
	factory.threading_mode = EThreadingMode::Single;
	pRHI->Init(&factory);

	return pRHI;
}
#endif
