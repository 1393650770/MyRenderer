#pragma once

#ifndef _VK_DEVICE_
#define _VK_DEVICE_
#include <vulkan/vulkan.h>

namespace MXRender
{
	class VK_Device
	{
	private:
	protected:
	public:
		VkDevice Device;
		VkPhysicalDevice Gpu= VK_NULL_HANDLE;
		VkPhysicalDeviceProperties GpuProps;
	};
}

#endif //
