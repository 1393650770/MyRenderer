
#include "VK_GraphicsContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include"VK_Device.h"
#include <stdexcept>
#include "../RenderPass.h"
#include "VK_DescriptorSets.h"
#include "VK_Viewport.h"
#include "VK_RenderPass.h"
#include "../../Render/Pass/MainCameraRenderPass.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

namespace MXRender
{

	

	void VK_GraphicsContext::create_instance()
	{
        if (enableValidationLayers && !check_validationlayer_support()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
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

    void VK_GraphicsContext::initialize_debugmessenger()
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populate_debug_messenger_createInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void VK_GraphicsContext::create_surface(GLFWwindow* window)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

 

    void VK_GraphicsContext::initialize_physical_device()
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

    }

    void VK_GraphicsContext::create_logical_device()
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

        if (vkCreateDevice(device->gpu, &createInfo, nullptr, &device->device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device->device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device->device, indices.presentFamily.value(), 0, &presentQueue);
    }

	void VK_GraphicsContext::create_command_pool()
	{
		QueueFamilyIndices queueFamilyIndices = find_queue_families(device->gpu);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		for (uint32_t i = 0; i < max_frames_in_flight; ++i)
		{
			if (vkCreateCommandPool(device->device, &poolInfo, nullptr, &command_pool[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics command pool!");
			}
        }
	}

	void VK_GraphicsContext::create_command_buffer()
	{

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		//alloc_info.commandPool = command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		for (uint32_t i = 0; i < max_frames_in_flight; ++i)
		{
            alloc_info.commandPool = command_pool[i];

			if (vkAllocateCommandBuffers(device->device, &alloc_info, &command_buffer[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
        }
	}

	void VK_GraphicsContext::create_sync_object()
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

		for (uint32_t i = 0; i < max_frames_in_flight; i++)
		{
		    if (vkCreateSemaphore(
			    device->device, &semaphore_create_info, nullptr, &image_available_for_render_semaphore[i]) !=
			    VK_SUCCESS ||
			    vkCreateSemaphore(
                    device->device, &semaphore_create_info, nullptr, &image_finished_for_presentation_semaphore[i]) !=
			    VK_SUCCESS ||
			    vkCreateFence(device->device, &fence_create_info, nullptr, &frame_in_flight_fence[i]) != VK_SUCCESS)
		    {
			    throw std::runtime_error("vk create semaphore & fence");
		    }
        }
		
	}

	std::vector<const char*> VK_GraphicsContext::get_required_extensions()
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

    bool VK_GraphicsContext::check_validationlayer_support()
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

    void VK_GraphicsContext::populate_debug_messenger_createInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    bool VK_GraphicsContext::check_device_suitable(VkPhysicalDevice device)
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

    QueueFamilyIndices VK_GraphicsContext::find_queue_families(VkPhysicalDevice device)
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

    bool VK_GraphicsContext::check_device_extension_support(VkPhysicalDevice device)
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

    SwapChainSupportDetails VK_GraphicsContext::query_swapchain_support(VkPhysicalDevice device)
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

	void VK_GraphicsContext::recreate_swapchain(VkSwapchainKHR& swapchain)
	{


	}

	VK_GraphicsContext::VK_GraphicsContext()
    {
        device=std::make_shared<VK_Device>();

    }

    VK_GraphicsContext::~VK_GraphicsContext()
    {



        vkDestroySurfaceKHR(instance, surface, nullptr);
		for (uint32_t i = 0; i < max_frames_in_flight; i++)
		{
			vkDestroySemaphore(device->device, image_finished_for_presentation_semaphore[i], nullptr);
			vkDestroySemaphore(device->device, image_available_for_render_semaphore[i], nullptr);
			vkDestroyFence(device->device, frame_in_flight_fence[i], nullptr);
		}
		for (uint32_t i = 0; i < max_frames_in_flight; i++)
		{
            vkDestroyCommandPool(device->device, command_pool[i], nullptr);
        }
		vkDestroyDevice(device->device, nullptr);
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);

		vkDestroyInstance(instance, nullptr);
     }

    void VK_GraphicsContext::init(GLFWwindow* window)
    {
        create_instance();
        initialize_debugmessenger();
        create_surface(window);
        initialize_physical_device();
        create_logical_device();
        create_command_pool();
        create_command_buffer();
        create_sync_object();

        descriptor_pool = std::make_shared<VK_DescriptorPool>(device, 16);
    }

    void VK_GraphicsContext::pre_init()
    {
    }

    std::shared_ptr<VK_Device> VK_GraphicsContext::get_device()
    {
        return  device;
    }


	std::shared_ptr<VK_DescriptorPool> VK_GraphicsContext::get_descriptor_pool()
	{
        return descriptor_pool;
	}

	VkInstance VK_GraphicsContext::get_instance()
	{
        return instance;
	}

	void VK_GraphicsContext::wait_for_fences()
	{
		VkResult res_wait_for_fences =vkWaitForFences(device->device, 1, &frame_in_flight_fence[current_frame_index], VK_TRUE, UINT64_MAX);

		if (VK_SUCCESS != res_wait_for_fences)
		{
			throw std::runtime_error("failed to synchronize");
		}
	}

	void VK_GraphicsContext::reset_commandbuffer()
	{
		VkResult res_reset_commandbuffer = vkResetCommandBuffer(command_buffer[current_frame_index],0);

		if (VK_SUCCESS != res_reset_commandbuffer)
		{
			throw std::runtime_error("failed to synchronize");
		}
	}

	VkSurfaceKHR VK_GraphicsContext::get_surface()
	{
        return surface;
	}

	VkCommandBuffer& VK_GraphicsContext::get_cur_command_buffer()
	{
        return command_buffer[current_frame_index];
	}


	uint8_t VK_GraphicsContext::get_current_frame_index() const
	{
        return current_frame_index;
	}

	void VK_GraphicsContext::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
	{

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = command_pool[0];
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src_buffer, dst_buffer, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device->device, command_pool[0], 1, &commandBuffer);
	}

	void VK_GraphicsContext::pre_pass( VkSwapchainKHR& swapchain, uint32_t& image_index)
	{
		VkResult result = vkAcquireNextImageKHR(device->device, swapchain, UINT64_MAX, image_available_for_render_semaphore[current_frame_index], VK_NULL_HANDLE, &image_index);


		if (VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			return ;
		}
		else if (VK_SUBOPTIMAL_KHR == result)
		{


			// NULL submit to wait semaphore
			VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
			VkSubmitInfo         submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitSemaphores = &image_available_for_render_semaphore[current_frame_index];
			submit_info.pWaitDstStageMask = wait_stages;
			submit_info.commandBufferCount = 0;
			submit_info.pCommandBuffers = NULL;
			submit_info.signalSemaphoreCount = 0;
			submit_info.pSignalSemaphores = NULL;

			wait_for_fences();

			VkResult res_queue_submit =
				vkQueueSubmit(graphicsQueue, 1, &submit_info, frame_in_flight_fence[current_frame_index]);
			assert(VK_SUCCESS == res_queue_submit);

			current_frame_index = (current_frame_index + 1) % max_frames_in_flight;
			return ;
		}
		else
		{
			assert(VK_SUCCESS == result);
		}

		// begin command buffer
		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = 0;
		command_buffer_begin_info.pInheritanceInfo = nullptr;

		VkResult res_begin_command_buffer =
            vkBeginCommandBuffer(command_buffer[current_frame_index], &command_buffer_begin_info);
	}



	void VK_GraphicsContext::submit(VkSwapchainKHR* swapchains, int swapchain_count, uint32_t& image_index)
	{
		VkResult res_end_command_buffer = vkEndCommandBuffer(command_buffer[current_frame_index]);
		assert(VK_SUCCESS == res_end_command_buffer);

		// submit command buffer
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo         submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &image_available_for_render_semaphore[current_frame_index];
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer[current_frame_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &image_finished_for_presentation_semaphore[current_frame_index];

		VkResult res_reset_fences = vkResetFences(device->device, 1, &frame_in_flight_fence[current_frame_index]);
		assert(VK_SUCCESS == res_reset_fences);

		VkResult res_queue_submit =
			vkQueueSubmit(graphicsQueue, 1, &submit_info, frame_in_flight_fence[current_frame_index]);
		assert(VK_SUCCESS == res_queue_submit);

		// present swapchain
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &image_finished_for_presentation_semaphore[current_frame_index];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;

		VkResult present_result = vkQueuePresentKHR(presentQueue, &present_info);
		if (VK_ERROR_OUT_OF_DATE_KHR == present_result || VK_SUBOPTIMAL_KHR == present_result)
		{
			//recreateSwapchain();
			//passUpdateAfterRecreateSwapchain();
		}
		else
		{
			assert(VK_SUCCESS == present_result);
		}

		current_frame_index = (current_frame_index + 1) % max_frames_in_flight;

	}

}
