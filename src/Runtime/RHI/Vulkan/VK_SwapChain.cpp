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
#include "VK_Platform.h"
#include "vulkan/vulkan_core.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

VK_SwapChain::VK_SwapChain(VkInstance in_instance, VK_Device* in_device, void* in_window_handle, ENUM_TEXTURE_FORMAT& in_out_pixel_format, Int width, Int height, Bool is_full_screen,
	UInt32* in_out_desired_num_back_buffers, Vector<VkImage>& out_images, Int in_lock_to_vsync, VK_SwapChainRecreateInfo* recreate_info) :
	swapchain(VK_NULL_HANDLE)
	, device(in_device)
	, surface(VK_NULL_HANDLE)
	, window_handle(in_window_handle)
	, current_image_index(-1)
	, semaphore_index(0)
	, num_present_calls(0)
	, num_acquire_calls(0)
	, instance(in_instance)
	, lock_to_vsync(in_lock_to_vsync)
{
	if (recreate_info != nullptr && recreate_info->swapchain != VK_NULL_HANDLE)
	{
		surface = recreate_info->surface;
		recreate_info->surface = VK_NULL_HANDLE;
	}
	else if (recreate_info != nullptr)
	{
		surface = recreate_info->surface;
		recreate_info->surface = VK_NULL_HANDLE;
	} 
	else
	{
		VK_Platform::CreateSurface(window_handle,instance,&surface);
	}

	unsigned int num_formats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetGpu(), surface, &num_formats, nullptr);
	if (num_formats <= 0)
	{
		return;
	}
	std::vector<VkSurfaceFormatKHR> Formats(num_formats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetGpu(), surface, &num_formats, Formats.data());

	VkColorSpaceKHR requested_colorspace = Formats[0].colorSpace;
	VkSurfaceFormatKHR cur_format = Formats[0];
	for (const auto& Format : Formats) {
		if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			cur_format = Format;
			requested_colorspace = Format.colorSpace;
			break;
		}
	} 

	VkSurfaceCapabilitiesKHR surf_properties;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetGpu(), surface, &surf_properties);

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
	UInt32 num_found_present_modes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetGpu(), surface, &num_found_present_modes, nullptr);;
	if (num_found_present_modes <= 0)
	{
		return;
	}

	Vector<VkPresentModeKHR> FoundPresentModes(num_found_present_modes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetGpu(), surface, &num_found_present_modes, FoundPresentModes.data());


	for (const auto& CurPresentMode : FoundPresentModes) {
		if (CurPresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = CurPresentMode;
			break;
		}
	}
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (surf_properties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}


	unsigned int desired_num_buffers = surf_properties.maxImageCount > 0 ? (*in_out_desired_num_back_buffers < surf_properties.minImageCount ? surf_properties.minImageCount : *in_out_desired_num_back_buffers < surf_properties.maxImageCount ? *in_out_desired_num_back_buffers : surf_properties.maxImageCount) : *in_out_desired_num_back_buffers;
	VkSwapchainCreateInfoKHR swapchain_info;
	swapchain_info.surface = surface;
	swapchain_info.minImageCount = desired_num_buffers;
	swapchain_info.imageFormat = cur_format.format;
	swapchain_info.imageColorSpace = cur_format.colorSpace;
	swapchain_info.imageExtent.width = width;
	swapchain_info.imageExtent.height = height;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_info.preTransform = pre_transform;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.presentMode = present_mode;
	swapchain_info.oldSwapchain = VK_NULL_HANDLE;
	swapchain_info.pNext = nullptr;
	swapchain_info.flags = 0;
	if (recreate_info != nullptr)
	{
		swapchain_info.oldSwapchain = recreate_info->swapchain;
	}

	swapchain_info.clipped = VK_TRUE;
	swapchain_info.compositeAlpha = composite_alpha;
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	*in_out_desired_num_back_buffers = desired_num_buffers;

	CHECK_WITH_LOG(vkCreateSwapchainKHR(device->GetDevice(), &swapchain_info, nullptr, &swapchain) != VK_SUCCESS,
					"RHI Error: failed to create swap chain!")


	vkGetSwapchainImagesKHR(device->GetDevice(), swapchain, &num_swap_chain_images, nullptr);
	out_images.resize(num_swap_chain_images);
	vkGetSwapchainImagesKHR(device->GetDevice(), swapchain, &num_swap_chain_images, out_images.data());

	image_format = cur_format.format;
	image_extent2D = swapchain_info.imageExtent;

	if(surface)
	{
		device->CreatePresentQueue(surface);
	}
}


VK_SwapChain::~VK_SwapChain()
{
	destroy(nullptr);
}

void VK_SwapChain::destroy(VK_SwapChainRecreateInfo* RecreateInfo)
{

	bool bRecreate = RecreateInfo != nullptr;
	if (bRecreate)
	{
		RecreateInfo->swapchain = swapchain;
		RecreateInfo->surface = surface;
	}
	else
	{

		vkDestroySwapchainKHR(device->GetDevice(), swapchain, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	swapchain = VK_NULL_HANDLE;
	surface = VK_NULL_HANDLE;
}

VkFormat VK_SwapChain::get_image_format() const
{
	return image_format;
}

VkSwapchainKHR& VK_SwapChain::get_swapchain()
{
	return swapchain;
}

VkExtent2D VK_SwapChain::get_extent2D() const
{
	return image_extent2D;
}

unsigned int VK_SwapChain::get_swap_chain_images_num() const
{
	return num_swap_chain_images;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

