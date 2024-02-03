#include "Platform/Platform.h"
#include "RHI/Vulkan/VK_RenderRHI.h"

MXRender::RHI::RenderRHI* PlatformCreateDynamicRHI()
{
	MXRender::RHI::Vulkan::VulkanRHI* pRHI = new MXRender::RHI::Vulkan::VulkanRHI();
	MXRender::RHI::Vulkan::VulkanRenderFactory factory;
	pRHI->Init(&factory);

	return pRHI;
}
