
#include "VK_GraphicsContext.h"
#include "VK_Device.h"
#include "VK_Utils.h"
#include "../../Render/DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "../../Render/Window.h"
#include "../../Logic/GameObjectManager.h"
#include "../../Mesh/MeshBase.h"
#include "../../Render/Pass/PipelineShaderObject.h"
#include <thread>
#include "../../Logic/TaskScheduler.h"

#define VMA_IMPLEMENTATION
#include "../../../ThirdParty/vma/vk_mem_alloc.h"
#include "../MyTexture.h"



void MXRender::VK_GraphicsContext::create_instance()
{
	if (enableValidationLayers && !check_validationlayer_support()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "MyRender";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "MyRender";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = get_required_extensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populate_debug_messenger_createInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void MXRender::VK_GraphicsContext::initialize_debugmessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populate_debug_messenger_createInfo(createInfo);

	if (create_debug_utils_messengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void MXRender::VK_GraphicsContext::create_surface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void MXRender::VK_GraphicsContext::initialize_physical_device()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& curdevice : devices) {
		if (check_device_suitable(curdevice)) {
			device->gpu = curdevice;
			break;
		}
	}

	if (device->gpu == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	
	vkGetPhysicalDeviceProperties(device->gpu, &(device->gpu_props));

}

void MXRender::VK_GraphicsContext::create_logical_device()
{
	QueueFamilyIndices indices = find_queue_families(device->gpu);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.multiDrawIndirect= VK_TRUE;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	//deviceFeatures.pipelineStatisticsQuery = VK_TRUE;
	//deviceFeatures.drawIndirectFirstInstance = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(device->gpu, &createInfo, nullptr, &(device->device)) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device->device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device->device, indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(device->device, indices.computeFamily.value(), 0, &computeQueue);






	depth_image_format = find_supported_depth_format({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void MXRender::VK_GraphicsContext::init_vma_allocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = device->gpu;
	allocatorInfo.device = device->device;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
}

void MXRender::VK_GraphicsContext::create_command_pool()
{
	QueueFamilyIndices queueFamilyIndices = find_queue_families(device->gpu);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device->device, &poolInfo, nullptr, &command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	thread_command_pool.resize(std::thread::hardware_concurrency());

	for (unsigned int i=0 ;i< thread_command_pool.size();i++)
	{
		if (vkCreateCommandPool(device->device, &poolInfo, nullptr, &thread_command_pool[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
}

void MXRender::VK_GraphicsContext::create_command_buffer()
{
	command_buffer.resize(max_frames_in_flight);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)command_buffer.size();

	if (vkAllocateCommandBuffers(device->device, &allocInfo, command_buffer.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	thread_command_buffer.resize(std::thread::hardware_concurrency());
	
	VkCommandBufferAllocateInfo secondary_allocInfo{};
	secondary_allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	secondary_allocInfo.commandBufferCount =1;
	secondary_allocInfo.level= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	for (unsigned int i = 0; i < thread_command_pool.size(); i++)
	{
		secondary_allocInfo.commandPool=thread_command_pool[i];
		if (vkAllocateCommandBuffers(device->device, &secondary_allocInfo, &thread_command_buffer[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}

void MXRender::VK_GraphicsContext::create_sync_object()
{


	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < max_frames_in_flight; i++) {
		if (vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &image_available_for_render_semaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &image_finished_for_presentation_semaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device->device, &fenceInfo, nullptr, &frame_in_flight_fence[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

uint32_t previousPow2(uint32_t v)
{
	uint32_t r = 1;

	while (r * 2 < v)
		r *= 2;

	return r;
}
uint32_t getImageMipLevels(uint32_t width, uint32_t height)
{
	uint32_t result = 1;

	while (width > 1 || height > 1)
	{
		result++;
		width /= 2;
		height /= 2;
	}

	return result;
}

void MXRender::VK_GraphicsContext::create_depth_pyramid_image()
{
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	depth_pyramid_width = previousPow2(swapchain_extent.width);
	depth_pyramid_height = previousPow2(swapchain_extent.height);
	depth_pyramid_levels = getImageMipLevels(depth_pyramid_width, depth_pyramid_height);

	VkExtent3D pyramidExtent = {
		static_cast<uint32_t>(depth_pyramid_width),
		static_cast<uint32_t>(depth_pyramid_height),
		1
	};
	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo pyramidInfo = VK_Utils::Image_Create_Info(VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, pyramidExtent);

	pyramidInfo.mipLevels = depth_pyramid_levels;

	//allocate and create the image
	vmaCreateImage(_allocator, &pyramidInfo, &dimg_allocinfo, &depth_pyramid_image._image, &depth_pyramid_image._allocation, nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo priview_info =VK_Utils::Imageview_Create_Info(VK_FORMAT_R32_SFLOAT, depth_pyramid_image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	priview_info.subresourceRange.levelCount = depth_pyramid_levels;


	vkCreateImageView(device->device, &priview_info, nullptr, &depth_pyramid_image._defaultView);


	for (int32_t i = 0; i < depth_pyramid_levels; ++i)
	{
		VkImageViewCreateInfo level_info = VK_Utils::Imageview_Create_Info(VK_FORMAT_R32_SFLOAT, depth_pyramid_image._image, VK_IMAGE_ASPECT_COLOR_BIT);
		level_info.subresourceRange.levelCount = 1;
		level_info.subresourceRange.baseMipLevel = i;

		VkImageView pyramid;
		vkCreateImageView(device->device, &level_info, nullptr, &pyramid);

		depth_pyramid_mips[i] = pyramid;
		assert(depth_pyramid_mips[i]);
	}





	VkSamplerCreateInfo createInfo = {};

	auto reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.minLod = 0;
	createInfo.maxLod = 16.f;

	VkSamplerReductionModeCreateInfoEXT createInfoReduction = { VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT };

	if (reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_EXT)
	{
		createInfoReduction.reductionMode = reductionMode;

		createInfo.pNext = &createInfoReduction;
	}


	vkCreateSampler(device->device, &createInfo, 0, &depth_sampler);


	VK_Utils::Immediate_Submit(this, [&](VkCommandBuffer cmd) {
			VK_Utils::Transition_ImageLayout(Singleton<DefaultSetting>::get_instance().context, depth_pyramid_image._image
			, VK_IMAGE_LAYOUT_UNDEFINED
			, VK_IMAGE_LAYOUT_GENERAL,1,depth_pyramid_levels, VK_IMAGE_ASPECT_COLOR_BIT);
		});
}

VkResult MXRender::VK_GraphicsContext::create_debug_utils_messengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void MXRender::VK_GraphicsContext::destroy_debug_utils_messengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

std::vector<const char*> MXRender::VK_GraphicsContext::get_required_extensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool MXRender::VK_GraphicsContext::check_validationlayer_support()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void MXRender::VK_GraphicsContext::populate_debug_messenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

bool MXRender::VK_GraphicsContext::check_device_suitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = find_queue_families(device);

	bool extensionsSupported = check_device_extension_support(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = query_swapchain_support(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

MXRender::QueueFamilyIndices MXRender::VK_GraphicsContext::find_queue_families(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

bool MXRender::VK_GraphicsContext::check_device_extension_support(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

MXRender::SwapChainSupportDetails MXRender::VK_GraphicsContext::query_swapchain_support(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkFormat MXRender::VK_GraphicsContext::find_supported_depth_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(device->gpu, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("findSupportedFormat failed");
}

VkSurfaceFormatKHR MXRender::VK_GraphicsContext::chooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& available_surface_formats)
{
	for (const auto& availableFormat : available_surface_formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return available_surface_formats[0];
}


VkPresentModeKHR MXRender::VK_GraphicsContext::chooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& availablePresentMode : available_present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MXRender::VK_GraphicsContext::chooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

MXRender::VK_GraphicsContext::VK_GraphicsContext()
{
	device=std::make_shared<VK_Device>();
}

MXRender::VK_GraphicsContext::~VK_GraphicsContext()
{
	cleanup();
}

void MXRender::VK_GraphicsContext::init(Window* new_window)
{
	window=new_window->GetWindow();
	create_instance();
	initialize_debugmessenger();
	create_surface();
	initialize_physical_device();
	create_logical_device();
	init_vma_allocator();
	create_swapchain();
	create_swapchain_imageviews();
	create_framebuffer_imageAndview();
	create_command_pool();
	create_command_buffer();
	create_sync_object();
	init_pass();
	create_depth_pyramid_image();
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = get_swapchain_extent().width;
	viewport.height = get_swapchain_extent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;


}

void MXRender::VK_GraphicsContext::pre_init()
{

}


VkInstance MXRender::VK_GraphicsContext::get_instance()
{
	return instance;
}

void MXRender::VK_GraphicsContext::wait_for_fences()
{
	VkResult res_wait_for_fences =
		vkWaitForFences(device->device, 1, &frame_in_flight_fence[current_frame_index], VK_TRUE, UINT64_MAX);
	if (VK_SUCCESS != res_wait_for_fences)
	{
		throw std::runtime_error("failed to synchronize");
	}
}

void MXRender::VK_GraphicsContext::reset_commandpool()
{
	VkResult res_reset_command_pool = vkResetCommandPool(device->device, command_pool, 0);
	if (VK_SUCCESS != res_reset_command_pool)
	{
		throw std::runtime_error("failed to synchronize");
	}

	for (unsigned int i = 0; i < thread_command_pool.size(); i++)
	{
		res_reset_command_pool = vkResetCommandPool(device->device, thread_command_pool[i], 0);
		if (VK_SUCCESS != res_reset_command_pool)
		{
			throw std::runtime_error("failed to synchronize");
		}
	}
}

void MXRender::VK_GraphicsContext::create_swapchain()
{
	SwapChainSupportDetails swapChainSupport = query_swapchain_support(device->gpu);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormatFromDetails(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapchainPresentModeFromDetails(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapchainExtentFromDetails(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT| VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT| VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	QueueFamilyIndices indices = find_queue_families(device->gpu);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(device->device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device->device, swapchain, &imageCount, nullptr);
	swapchain_images.resize(imageCount);
	vkGetSwapchainImagesKHR(device->device, swapchain, &imageCount, swapchain_images.data());

	swapchain_image_format = surfaceFormat.format;
	swapchain_extent = extent;
}

void MXRender::VK_GraphicsContext::recreate_swapchain()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) // minimized 0,0, pause for now
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	VkResult res_wait_for_fences =
		vkWaitForFences(device->device, max_frames_in_flight, frame_in_flight_fence, VK_TRUE, UINT64_MAX);
	assert(VK_SUCCESS == res_wait_for_fences);

	clean_swapchain();
	create_swapchain();
	create_swapchain_imageviews();
	create_framebuffer_imageAndview();

	for (auto& func : on_swapchain_recreate)
	{
		func();
	}

	Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_width(width);
	Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.set_width(height);
}

void MXRender::VK_GraphicsContext::clean_swapchain()
{

	vkDestroyImageView(device->device, depth_image_view, nullptr);
	vkDestroyImage(device->device, depth_image, nullptr);
	vkFreeMemory(device->device, depth_image_memory, nullptr);

	for (auto& func:on_swapchain_clean)
	{
		func();
	}

	for (auto imageView : swapchain_imageviews) {
		vkDestroyImageView(device->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device->device, swapchain, nullptr);
}

void MXRender::VK_GraphicsContext::create_swapchain_imageviews()
{
	swapchain_imageviews.resize(swapchain_images.size());

	for (size_t i = 0; i < swapchain_images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchain_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain_image_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device->device, &createInfo, nullptr, &swapchain_imageviews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void MXRender::VK_GraphicsContext::create_framebuffer_imageAndview()
{
	VK_Utils::Create_Image(device->gpu,
		device->device,
		swapchain_extent.width,
		swapchain_extent.height,
		depth_image_format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
		 VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depth_image,
		depth_image_memory,
		0,
		1,
		1);

	depth_image_view = VK_Utils::Create_ImageView(
		device->device, depth_image, depth_image_format, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
}

VkSurfaceKHR MXRender::VK_GraphicsContext::get_surface()
{
	return surface;
}

VkCommandBuffer MXRender::VK_GraphicsContext::get_cur_command_buffer()
{
	return command_buffer[current_frame_index];
}

VkCommandBuffer MXRender::VK_GraphicsContext::get_cur_threadid_command_buffer(unsigned int thread_id)
{
	return thread_command_buffer[thread_id];
}

void MXRender::VK_GraphicsContext::reset_all_threadid_command_buffer()
{
	for (unsigned int i=0;i<thread_command_buffer.size();i++)
	{
		vkResetCommandBuffer(thread_command_buffer[i], /*VkCommandBufferResetFlagBits*/ 0);
	}
}

void MXRender::VK_GraphicsContext::begin_all_threadid_command_buffer()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	for (unsigned int i = 0; i < thread_command_buffer.size(); i++)
	{
		if (vkBeginCommandBuffer(thread_command_buffer[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}
}

void MXRender::VK_GraphicsContext::end_all_threadid_command_buffer()
{
	for (unsigned int i = 0; i < thread_command_buffer.size(); i++)
	{
		//if(thread_command_buffer_use_map[thread_command_buffer[i]])
		//{ 
			if (vkEndCommandBuffer(thread_command_buffer[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}
		/*}*/
	}
}	

void MXRender::VK_GraphicsContext::execute_all_threadid_command_buffer()
{
	for (unsigned int i = 0; i < thread_command_buffer.size(); i++)
	{
		vkCmdExecuteCommands(get_cur_command_buffer(), 1, &thread_command_buffer[i]);
	}
}

void MXRender::VK_GraphicsContext::submit_all_threadid_command_buffer()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//for (unsigned int i = 0; i < thread_command_buffer.size(); i++)
	//{
	//	if (thread_command_buffer_use_map[thread_command_buffer[i]])
	//	{
			submitInfo.commandBufferCount = thread_command_buffer.size();
			submitInfo.pCommandBuffers = thread_command_buffer.data();
			vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
			//thread_command_buffer_use_map[thread_command_buffer[i]] = 0;
	//	}
	//}


	vkQueueWaitIdle(graphicsQueue);
}

void MXRender::VK_GraphicsContext::wait_all_task()
{
	while (fut_que.empty() == false)
	{
		fut_que.front().wait();
		fut_que.pop();
	}
}

uint8_t MXRender::VK_GraphicsContext::get_current_frame_index() const
{
	return current_frame_index;
}

uint8_t MXRender::VK_GraphicsContext::get_max_frame_num() const
{
	return max_frames_in_flight;
}

uint32_t MXRender::VK_GraphicsContext::get_current_swapchain_image_index() const
{
	return current_swapchain_image_index;
}

VkFormat MXRender::VK_GraphicsContext::get_swapchain_image_format() const
{
	return swapchain_image_format;
}

VkFormat MXRender::VK_GraphicsContext::get_depth_image_format() const
{
	return depth_image_format;
}

VkExtent2D MXRender::VK_GraphicsContext::get_swapchain_extent() const
{
	return swapchain_extent;
}

VkImageView MXRender::VK_GraphicsContext::get_depth_image_view() const
{
	return depth_image_view;
}

VkImage MXRender::VK_GraphicsContext::get_depth_image() const
{
	return depth_image;
}

VkImage MXRender::VK_GraphicsContext::get_cur_swapchain_image()
{
	return swapchain_images[get_current_swapchain_image_index()];
}

GLFWwindow* MXRender::VK_GraphicsContext::get_window() const
{
	return window;
}

MXRender::QueueFamilyIndices MXRender::VK_GraphicsContext::get_queuefamily() 
{
	return find_queue_families(device->gpu);
}

void MXRender::VK_GraphicsContext::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = begin_single_time_commands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

	end_single_time_commands(commandBuffer);


}

VkCommandBuffer MXRender::VK_GraphicsContext::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void MXRender::VK_GraphicsContext::end_single_time_commands(VkCommandBuffer command_buffer)
{
	vkEndCommandBuffer(command_buffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device->device, command_pool, 1, &command_buffer);
}

void MXRender::VK_GraphicsContext::add_on_swapchain_recreate_func(const std::function<void()>& func)
{
	on_swapchain_recreate.emplace_back(func);
}

void MXRender::VK_GraphicsContext::add_on_swapchain_clean_func(const std::function<void()>& func)
{
	on_swapchain_clean.emplace_back(func);
}

void MXRender::VK_GraphicsContext::add_on_shutdown_clean_func(const std::function<void()>& func)
{
	on_shutdown_clean.emplace_back(func);
}

void MXRender::VK_GraphicsContext::pre_pass()
{
	vkWaitForFences(device->device, 1, &frame_in_flight_fence[current_frame_index], VK_TRUE, UINT64_MAX);
	vkResetFences(device->device, 1, &frame_in_flight_fence[current_frame_index]);
	vkQueueWaitIdle(graphicsQueue);

	VkResult result = vkAcquireNextImageKHR(device->device, swapchain, UINT64_MAX, image_available_for_render_semaphore[current_frame_index], VK_NULL_HANDLE, &current_swapchain_image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}


	reset_all_threadid_command_buffer();
	vkResetCommandBuffer(command_buffer[current_frame_index], /*VkCommandBufferResetFlagBits*/ 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_all_threadid_command_buffer();
	if (vkBeginCommandBuffer(command_buffer[current_frame_index], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

}



void MXRender::VK_GraphicsContext::init_pass()
{
	{

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = get_swapchain_image_format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = get_depth_image_format();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, &mesh_pass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = get_swapchain_image_format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = get_depth_image_format();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device->device, &renderPassInfo, nullptr, &clear_pass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}


}

void MXRender::VK_GraphicsContext::submit()
{
	if (vkEndCommandBuffer(command_buffer[current_frame_index]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
	wait_all_task();

	end_all_threadid_command_buffer();
	submit_all_threadid_command_buffer();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { image_available_for_render_semaphore[current_frame_index] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer[current_frame_index];

	VkSemaphore signalSemaphores[] = { image_finished_for_presentation_semaphore[current_frame_index] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;



	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frame_in_flight_fence[current_frame_index]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}


	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &current_swapchain_image_index;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	

	current_frame_index = (current_frame_index + 1) % max_frames_in_flight;

}


void MXRender::VK_GraphicsContext::cleanup()
{
	clean_swapchain();
	


	for (size_t i = 0; i < max_frames_in_flight; i++) {
		vkDestroySemaphore(device->device, image_finished_for_presentation_semaphore[i], nullptr);
		vkDestroySemaphore(device->device, image_available_for_render_semaphore[i], nullptr);
		vkDestroyFence(device->device, frame_in_flight_fence[i], nullptr);
	}

	vkDestroyCommandPool(device->device, command_pool, nullptr);
	//for (auto& it : on_shutdown_clean)
	//{
	//	it();
	//}
	for (unsigned int i = 0; i < thread_command_pool.size(); i++)
	{
		vkDestroyCommandPool(device->device, thread_command_pool[i], nullptr);
	}

	vmaDestroyAllocator(_allocator);
	vkDestroyDevice(device->device, nullptr);

	if (enableValidationLayers) {
		destroy_debug_utils_messengerEXT(instance, debug_messenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

MXRender::AllocatedBufferUntyped MXRender::VK_GraphicsContext::create_allocate_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags required_flags)
{
		//allocate vertex buffer
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.size = allocSize;

		bufferInfo.usage = usage;


		//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;
		vmaallocInfo.requiredFlags = required_flags;
		AllocatedBufferUntyped newBuffer;

		vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
			&newBuffer._buffer,
			&newBuffer._allocation,
			nullptr);
		newBuffer._size = allocSize;
		return newBuffer;
	
}

MXRender::AllocatedImage MXRender::VK_GraphicsContext::upload_allocate_image(VK_GraphicsContext* context, int texWidth, int texHeight, VkFormat image_format, AllocatedBufferUntyped& stagingBuffer, std::vector<MipmapInfo> mips)
{

		VkExtent3D imageExtent;
		imageExtent.width = static_cast<uint32_t>(texWidth);
		imageExtent.height = static_cast<uint32_t>(texHeight);
		imageExtent.depth = 1;

		VkImageCreateInfo dimg_info = VK_Utils::Image_Create_Info(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

		dimg_info.mipLevels = (uint32_t)mips.size();

		AllocatedImage newImage;

		VmaAllocationCreateInfo dimg_allocinfo = {};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		//allocate and create the image
		vmaCreateImage(context->_allocator, &dimg_info, &dimg_allocinfo, &newImage._image, &newImage._allocation, nullptr);

		//transition image to transfer-receiver	
		VK_Utils::Immediate_Submit(context, [&](VkCommandBuffer cmd) {
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = (uint32_t)mips.size();
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer = {};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = newImage._image;
			imageBarrier_toTransfer.subresourceRange = range;

			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//barrier the image into the transfer-receive layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

			for (int i = 0; i < mips.size(); i++) {



				VkBufferImageCopy copyRegion = {};
				copyRegion.bufferOffset = mips[i].dataOffset;
				copyRegion.bufferRowLength = 0;
				copyRegion.bufferImageHeight = 0;

				copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.imageSubresource.mipLevel = i;
				copyRegion.imageSubresource.baseArrayLayer = 0;
				copyRegion.imageSubresource.layerCount = 1;
				copyRegion.imageExtent = imageExtent;

				//copy the buffer into the image
				vkCmdCopyBufferToImage(cmd, stagingBuffer._buffer, newImage._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

				imageExtent.width /= 2;
				imageExtent.height /= 2;
			}
			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//barrier the image into the shader readable layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
			});



		newImage.mipLevels = (uint32_t)mips.size();


		//build a default imageview
		VkImageViewCreateInfo view_info =VK_Utils::Imageview_Create_Info(image_format, newImage._image, VK_IMAGE_ASPECT_COLOR_BIT);
		view_info.subresourceRange.levelCount = newImage.mipLevels;
		vkCreateImageView(context->device->device, &view_info, nullptr, &newImage._defaultView);

		context->add_on_shutdown_clean_func([=, &context]() {
			vmaDestroyImage(context->_allocator, newImage._image, newImage._allocation);
			});

		return newImage;
	
}

void* MXRender::VK_GraphicsContext::map_allocate_buffer(AllocatedBufferUntyped& buffer)
{
	void* data;
	vmaMapMemory(_allocator, buffer._allocation, &data);
	return data;
}

void MXRender::VK_GraphicsContext::unmap_allocate_buffer(AllocatedBufferUntyped& buffer)
{
	vmaUnmapMemory(_allocator, buffer._allocation);
}

void MXRender::VK_GraphicsContext::destroy_allocate_buffer(AllocatedBufferUntyped& buffer)
{
	vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
}
