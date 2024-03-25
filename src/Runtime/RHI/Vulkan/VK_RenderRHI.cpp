
#include "VK_RenderRHI.h"

#include "VK_Device.h"
#define  GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VK_Buffer.h"
#include "VK_Viewport.h"
#include "VK_Shader.h"
#include <iostream>
#include "VK_Texture.h"
#include "VK_RenderPass.h"
#include "VK_FrameBuffer.h"
#include "VK_PipelineState.h"
#include "VK_CommandBuffer.h"
#include "VK_Queue.h"
#include "vulkan/vulkan_core.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

CONST Vector<CONST Char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

CONST Vector<CONST Char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	"VK_EXT_sampler_filter_minmax"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << messageSeverity << " " << messageType << " " << pCallbackData->pMessage << std::endl;
	CHECK(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT == messageSeverity && messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	return VK_FALSE;
}

void VulkanRHI::Init(RenderFactory* render_factory)
{
	VulkanRenderFactory* vulkan_render_factory = STATIC_CAST(render_factory,VulkanRenderFactory);
	Bool enable_render_debug = vulkan_render_factory->enable_render_debug;
	CreateInstance(enable_render_debug);
	InitializeDebugmessenger(enable_render_debug);
	CreateDevice(enable_render_debug);

	immediate_command_buffer = device->GetCommandBufferManager()->GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE::GRAPHICS);
}

void VulkanRHI::PostInit()
{

}


void VulkanRHI::Shutdown()
{

	if (viewports.size() > 0)
	{
		for (auto& viewport : viewports)
		{
			delete viewport;
		}
		viewports.clear();
	}
	if (defered_command_buffers.size() > 0)
	{
		for (auto& command_buffer : defered_command_buffers)
		{
			delete command_buffer;
		}
		defered_command_buffers.clear();
	}
	if (debug_messenger != VK_NULL_HANDLE)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debug_messenger, nullptr);
		}
		debug_messenger = VK_NULL_HANDLE;
	}
	if (device != nullptr)
	{
		delete device;
		device = nullptr;
	}
	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, nullptr);
		instance = VK_NULL_HANDLE;
	}
}


Buffer* VulkanRHI::CreateBuffer(const BufferDesc& buffer_desc)
{
	return new VK_Buffer(device,buffer_desc);
}

Texture* VulkanRHI::CreateTexture(const TextureDesc& texture_desc)
{
	return new VK_Texture(device,texture_desc);
}

RenderPipelineState* VulkanRHI::CreateRenderPipelineState(CONST RenderGraphiPipelineStateDesc& desc)
{
	Vector<ENUM_TEXTURE_FORMAT> rtv_formats;
	for (auto& rtv : desc.render_targets)
	{
		rtv_formats.push_back(rtv->GetTextureDesc().format);
	}
	RenderPassCacheKey key(desc.render_targets.size(), rtv_formats.data(), desc.depth_stencil_view->GetTextureDesc().format, desc.depth_stencil_view->GetTextureDesc().samples, false, false);
	return device->GetPipelineStateManager()->GetPipelineState(desc, device->GetRenderPassManager()->GetRenderPass(key));
}
RenderPass* VulkanRHI::CreateRenderPass(CONST RenderPassDesc& desc)
{
	return device->GetRenderPassManager()->GetRenderPass(desc);
}
FrameBuffer* VulkanRHI::CreateFrameBuffer(CONST FrameBufferDesc& desc)
{
	Vector<ENUM_TEXTURE_FORMAT> rtv_formats;
	for (auto& rtv : desc.render_targets)
	{
		rtv_formats.push_back(rtv->GetTextureDesc().format);
	}
	RenderPassCacheKey key(desc.render_targets.size(), rtv_formats.data(), desc.depth_stencil_view->GetTextureDesc().format, desc.depth_stencil_view->GetTextureDesc().samples, false, false);
	return new VK_FrameBuffer(device,desc, device->GetRenderPassManager()->GetRenderPass(key)->GetRenderPass());
}

void* VulkanRHI::MapBuffer(Buffer* buffer)
{
	return buffer->Map();
}

void VulkanRHI::UnmapBuffer(Buffer* buffer)
{
	return buffer->Unmap();
}

Bool VulkanRHI::CheckGpuSuitable(VkPhysicalDevice gpu)
{
	//TODO: Check Queue Family Suitable
	
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

	Set<String> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (CONST auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


VkPhysicalDevice VulkanRHI::GetGpuFromHarddrive()
{
	VkPhysicalDevice gpu=VK_NULL_HANDLE;
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

	if (device_count == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> gpus(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, gpus.data());

	for (const auto& cur_gpu : gpus) {
		if (CheckGpuSuitable(cur_gpu)) {
			gpu= cur_gpu;
			break;
		}
	}
	CHECK_WITH_LOG(gpu==VK_NULL_HANDLE,"RHI Error: failed to find a suitable GPU!");
	return gpu;
}

void VulkanRHI::CreateDevice(Bool enable_validation_layers)
{
	if(device==nullptr)
	{
		VkPhysicalDevice physicalDevice =  GetGpuFromHarddrive();
		CHECK_WITH_LOG(physicalDevice==VK_NULL_HANDLE,"RHI Error: failed to find a suitable GPU!");
		device= new  VK_Device(this,physicalDevice);
		device->Init(0,enable_validation_layers,deviceExtensions,validationLayers);
	}
}


Vector<CONST Char*> VulkanRHI::GetRequiredExtensions(Bool enable_validation_layers)
{
	uint32_t glfwExtensionCount = 0;
	CONST Char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<CONST Char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enable_validation_layers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

Bool VulkanRHI::CheckValidationlayerSupport()
{
	UInt32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	Vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (CONST Char* layerName : validationLayers) {
		Bool layerFound = false;

		for (CONST auto& layerProperties : availableLayers) {
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

void VulkanRHI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
	create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debugCallback;
}

void VulkanRHI::CreateInstance(Bool enable_validation_layers)
{
	if (enable_validation_layers && !CheckValidationlayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "MyRender";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "MyRender";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &app_info;

	auto extensions = GetRequiredExtensions(enable_validation_layers);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (enable_validation_layers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debug_create_info);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanRHI::InitializeDebugmessenger(Bool enable_validation_layers)
{
	if (!enable_validation_layers) 
		return;

	VkDebugUtilsMessengerCreateInfoEXT create_info;
	PopulateDebugMessengerCreateInfo(create_info);

	if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult VulkanRHI::CreateDebugUtilsMessengerEXT(VkInstance instance, CONST VkDebugUtilsMessengerCreateInfoEXT* create_info, CONST VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debug_messenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, create_info, allocator, debug_messenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

Viewport* VulkanRHI::CreateViewport(void* window_handle, Int width, Int height, Bool is_full_screen)
{
	return new VK_Viewport(this, device, window_handle, width, height, is_full_screen,ENUM_TEXTURE_FORMAT::BGRA8);
}

Shader* VulkanRHI::CreateShader(CONST ShaderDesc& desc, CONST ShaderDataPayload& data)
{
	return new VK_Shader(device,desc,data);
}

CommandList* VulkanRHI::GetImmediateCommandList()
{
	immediate_command_buffer->Begin();
	return immediate_command_buffer;
}

void VulkanRHI::SubmitCommandList(CommandList* command_list)
{
	device->GetQueue(ENUM_QUEUE_TYPE::GRAPHICS)->Submit(STATIC_CAST(command_list,VK_CommandBuffer));
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE