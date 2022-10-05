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
		VkDevice device;
		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties gpu_props;
	};
}

#endif //
