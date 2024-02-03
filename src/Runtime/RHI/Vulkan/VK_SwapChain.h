#pragma once

#ifndef _VK_SWAPCHAIN_
#define _VK_SWAPCHAIN_

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include "RHI/RenderEnum.h"
#include "RHI/RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_STRUCT(VK_SwapChainRecreateInfo)
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkSurfaceKHR surface{};
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_SwapChain,public RenderResource)

#pragma region METHOD
public:
	VK_SwapChain(VkInstance in_instance, VK_Device* in_device, void* in_window_handle, ENUM_TEXTURE_FORMAT& in_out_pixel_format, Int width, Int height, Bool is_full_screen,
		UInt32* in_out_desired_num_back_buffers, Vector<VkImage>& out_images, Int in_lock_to_vsync, VK_SwapChainRecreateInfo* recreate_info);
	VIRTUAL ~VK_SwapChain();

	void METHOD(Destroy)(VK_SwapChainRecreateInfo* RecreateInfo);

	VkFormat METHOD(GetImageFormat)() CONST;

	VkSwapchainKHR& METHOD(GetSwapchain)();
	VkExtent2D METHOD(GetExtent2D)() CONST;
	UInt32 METHOD(GetSwapChainImagesNum)() CONST;

#pragma endregion


#pragma region MEMBER

private:
protected:
	VkSurfaceTransformFlagBitsKHR qcom_render_pass_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	VkFormat image_format = VK_FORMAT_UNDEFINED;
	VkExtent2D image_extent2D;
	VkSwapchainKHR swapchain;
	VK_Device* device;

	VkSurfaceKHR surface;
	void* window_handle;

	int current_image_index;
	int semaphore_index;
	int num_present_calls;
	int num_acquire_calls;
	int internal_width = 0;
	int internal_height = 0;
	bool b_internal_fullscreen = false;

	int RT_pacing_sample_count = 0;

	double rt_pacing_previous_frameCPUtime = 0;
	double rt_pacing_sampled_deltatimeMS = 0;

	double next_present_target_time = 0;

	VkInstance instance;

	int lock_to_vsync;

	int present_ID = 0;
	unsigned int num_swap_chain_images;

#pragma endregion





MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //
