#if PLATFORM_ANDROID
#include "Platform/Platform.h"
#include "RHI/Vulkan/VK_RenderRHI.h"

MXRender::RHI::RenderRHI* PlatformCreateDynamicRHI()
{
	MXRender::RHI::Vulkan::VulkanRHI* pRHI = new MXRender::RHI::Vulkan::VulkanRHI();
	MXRender::RHI::Vulkan::VulkanRenderFactory factory;
	factory.enable_render_debug = false;
	factory.threading_mode = EThreadingMode::Single; // Phase 1: single-threaded
	pRHI->Init(&factory);

	return pRHI;
}
#endif
