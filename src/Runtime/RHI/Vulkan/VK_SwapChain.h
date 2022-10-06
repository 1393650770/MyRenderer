#pragma once

#ifndef _VK_SWAPCHAIN_
#define _VK_SWAPCHAIN_

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "../RenderEnum.h"

namespace MXRender
{
    class VK_Device;
	struct VK_SwapChainRecreateInfo
	{
		VkSwapchainKHR swapchain;
		VkSurfaceKHR surface;
	};

	class VK_SwapChain
	{
	private:
	protected:
		VkSurfaceTransformFlagBitsKHR QCOMRenderPassTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		VkFormat image_format = VK_FORMAT_UNDEFINED;
		VkExtent2D Image_extent2D;
		VkSwapchainKHR swapchain;
		std::weak_ptr<VK_Device> device;

		VkSurfaceKHR surface;

		int current_image_index;
		int semaphore_index;
		int num_present_calls;
		int num_acquire_calls;
		int internal_width = 0;
		int internal_height = 0;
		bool b_internal_fullscreen = false;

		int RT_pacing_sample_count = 0;
		double RT_pacing_previous_frameCPUtime = 0;
		double RT_pacing_sampled_deltatimeMS = 0;

		double next_present_target_time = 0;

		VkInstance instance;

		int lock_to_vsync;

		int present_ID = 0;


	public:
		VK_SwapChain(VkInstance InInstance, std::shared_ptr<VK_Device> InDevice, void* WindowHandle, ENUM_TEXTURE_FORMAT& InOutPixelFormat, int Width, int Height, bool bIsFullscreen,
			int* InOutDesiredNumBackBuffers, std::vector<VkImage>& OutImages, int InLockToVsync, VK_SwapChainRecreateInfo* RecreateInfo);
		virtual ~VK_SwapChain();

		void destroy(VK_SwapChainRecreateInfo* RecreateInfo);
		VkFormat get_image_format() const;
		VkSwapchainKHR get_swapchain() const;

	};
}

#endif //
