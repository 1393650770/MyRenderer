#include "Vk_Device.h"
#include <iostream>
#include "VK_Queue.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

QueueFamilyIndices VK_Device::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.computeFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transferFamily = i;
		}

		//VkBool32 presentSupport = false;
		//vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		//if (presentSupport) {
		//	indices.presentFamily = i;
		//}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

void VK_Device::CreateDevice(bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers)
{
	QueueFamilyIndices indices = FindQueueFamilies(gpu);

	Vector<VkDeviceQueueCreateInfo> queue_create_infos;
	Set<UInt32> unique_queue_families = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (UInt32   queueFamily : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queueFamily;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queuePriority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features{};
	device_features.multiDrawIndirect = VK_TRUE;
	device_features.samplerAnisotropy = VK_TRUE;
	//deviceFeatures.pipelineStatisticsQuery = VK_TRUE;
	//deviceFeatures.drawIndirectFirstInstance = VK_TRUE;

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	create_info.pQueueCreateInfos = queue_create_infos.data();

	create_info.pEnabledFeatures = &device_features;

	create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();

	if (enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}
	else {
		create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(gpu, &create_info, nullptr, &(device)) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}


	CreateQueue(indices);
}



void VK_Device::GetDeviceExtensionsAndLayers(VkPhysicalDevice Gpu, UInt32 VendorId, Vector<CONST Char*>& OutDeviceExtensions, Vector<CONST Char*>& OutDeviceLayers, Vector<String>& OutAllDeviceExtensions, Vector<String>& OutAllDeviceLayers, bool& bOutDebugMarkers)
{


}
void VK_Device::CreateQueue(QueueFamilyIndices family_indice)
{
	graph_queue=new VK_Queue(this,family_indice.graphicsFamily.value());
	compute_queue = new VK_Queue(this, family_indice.computeFamily.value());
	present_queue = new VK_Queue(this, family_indice.presentFamily.value());
}

void VK_Device::Init(int device_index,bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers)
{
	CreateDevice(enable_validation_layers,device_extensions,validation_layers);
}

VK_Device::~VK_Device()
{

}

VK_Device::VK_Device(VulkanRHI* in_vulkan_rhi, VkPhysicalDevice in_gpu):
	vulkan_rhi(in_vulkan_rhi),
	gpu(in_gpu)
{
	vkGetPhysicalDeviceProperties(gpu,&gpu_props);
	vendor_id=gpu_props.vendorID;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE