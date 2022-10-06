#include "VK_SwapChain.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <set>
#include<memory>
#include <stdexcept>
#include "VK_Device.h"
#include "vulkan/vulkan_core.h"

namespace MXRender
{ 
	VK_SwapChain::VK_SwapChain(VkInstance InInstance, std::shared_ptr<VK_Device> InDevice, void* WindowHandle, ENUM_TEXTURE_FORMAT& InOutPixelFormat, int Width, int Height, bool bIsFullscreen, int* InOutDesiredNumBackBuffers, std::vector<VkImage>& OutImages, int InLockToVsync, VK_SwapChainRecreateInfo* RecreateInfo):
		  swapchain(VK_NULL_HANDLE)
		, device(InDevice)
		, surface(VK_NULL_HANDLE)
		, current_image_index(-1)
		, semaphore_index(0)
		, num_present_calls(0)
		, num_acquire_calls(0)
		, instance(InInstance)
		, lock_to_vsync(InLockToVsync)
	{

		if (device.expired())
		{
			return;
		}
		if (RecreateInfo != nullptr && RecreateInfo->swapchain != VK_NULL_HANDLE)
		{
			surface = RecreateInfo->surface;
			RecreateInfo->surface = VK_NULL_HANDLE;
		}
		else if (RecreateInfo != nullptr )
		{
			surface = RecreateInfo->surface;
			RecreateInfo->surface = VK_NULL_HANDLE;
		}
		else
		{
			GLFWwindow* windows= static_cast<GLFWwindow*>(WindowHandle);
			if (glfwCreateWindowSurface(instance, windows, nullptr, &surface) != VK_SUCCESS) {
				throw std::runtime_error("failed to create window surface!");
			}
		}

		unsigned int num_formats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.lock()->gpu, surface, &num_formats, nullptr);
		if (num_formats <= 0)
		{
			return;
		}
		std::vector<VkSurfaceFormatKHR> Formats(num_formats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.lock()->gpu, surface, &num_formats, Formats.data());

		VkColorSpaceKHR requested_colorspace = Formats[0].colorSpace;
		VkSurfaceFormatKHR CurrFormat= Formats[0];
		for (const auto& Format : Formats) {
			if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				CurrFormat = Format;
				requested_colorspace=Format.colorSpace;
				break;
			}
		}

		VkSurfaceCapabilitiesKHR surf_properties;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.lock()->gpu,surface,&surf_properties);

		VkSurfaceTransformFlagBitsKHR pre_transform;
		if (surf_properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			pre_transform = surf_properties.currentTransform;
		}


		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		unsigned int num_found_present_modes = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.lock()->gpu, surface, &num_found_present_modes, nullptr);;
		if (num_found_present_modes <= 0)
		{
			return;
		}

		std::vector<VkPresentModeKHR> FoundPresentModes(num_found_present_modes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.lock()->gpu, surface, &num_found_present_modes, FoundPresentModes.data());


		for (const auto& CurPresentMode : FoundPresentModes) {
			if (CurPresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				present_mode= CurPresentMode;
				break;
			}
		}
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		if (surf_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		{
			composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		}


		unsigned int desired_num_buffers = surf_properties.maxImageCount > 0 ? (*InOutDesiredNumBackBuffers< surf_properties.minImageCount ? surf_properties.minImageCount: *InOutDesiredNumBackBuffers< surf_properties.maxImageCount? *InOutDesiredNumBackBuffers: surf_properties.maxImageCount) : *InOutDesiredNumBackBuffers;
		VkSwapchainCreateInfoKHR swapchain_info;
		swapchain_info.surface = surface;
		swapchain_info.minImageCount = desired_num_buffers;
		swapchain_info.imageFormat = CurrFormat.format;
		swapchain_info.imageColorSpace = CurrFormat.colorSpace;
		swapchain_info.imageExtent.width = Width;
		swapchain_info.imageExtent.height = Height;
		swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapchain_info.preTransform = pre_transform;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.presentMode = present_mode;
		swapchain_info.oldSwapchain = VK_NULL_HANDLE;
		swapchain_info.pNext=nullptr;

		if (RecreateInfo != nullptr)
		{
			swapchain_info.oldSwapchain = RecreateInfo->swapchain;
		}

		swapchain_info.clipped = VK_TRUE;
		swapchain_info.compositeAlpha = composite_alpha;
		swapchain_info.sType=  VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		*InOutDesiredNumBackBuffers = desired_num_buffers;

		if (vkCreateSwapchainKHR(device.lock()->device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		unsigned int num_swap_chain_images;
		vkGetSwapchainImagesKHR(device.lock()->device, swapchain, &num_swap_chain_images, nullptr);
		OutImages.resize(num_swap_chain_images);
		vkGetSwapchainImagesKHR(device.lock()->device, swapchain, &num_swap_chain_images, OutImages.data());

		image_format = CurrFormat.format;
		Image_extent2D = swapchain_info.imageExtent;
	}


	VK_SwapChain::~VK_SwapChain()
	{
		destroy(nullptr);
	}

	void VK_SwapChain::destroy(VK_SwapChainRecreateInfo* RecreateInfo)
	{
		if (device.expired())
		{
			return;
		}
		bool bRecreate = RecreateInfo!=nullptr ;
		if (bRecreate)
		{
			RecreateInfo->swapchain = swapchain;
			RecreateInfo->surface = surface;
		}
		else
		{

			vkDestroySwapchainKHR(device.lock()->device, swapchain, nullptr);

			vkDestroySurfaceKHR(instance, surface, nullptr);
		}
		swapchain = VK_NULL_HANDLE;
		surface = VK_NULL_HANDLE;
	}

	VkFormat VK_SwapChain::get_image_format() const
	{
		return image_format;
	}

	VkSwapchainKHR VK_SwapChain::get_swapchain() const
	{
		return swapchain;
	}

}
