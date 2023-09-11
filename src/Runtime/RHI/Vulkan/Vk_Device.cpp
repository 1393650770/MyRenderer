#include "VK_Device.h"
#include <iostream>
#include <utility>

#include "VK_Fence.h"
#include "VK_Queue.h"
#include "VK_Memory.h"
#include "VK_Define.h"
#include "VK_CommandBuffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

QueueFamilyIndices VK_Device::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	UInt32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	Vector<VkQueueFamilyProperties> queueFamilies(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queueFamilies.data());

	Int i = 0;
	for (CONST auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.compute_family = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transfer_family = i;
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

void VK_Device::CreateDevice(Bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers)
{
	queue_family_indices = FindQueueFamilies(gpu);

	Vector<VkDeviceQueueCreateInfo> queue_create_infos;
	Set<UInt32> unique_queue_families = { queue_family_indices.graphics_family.value(), queue_family_indices.compute_family.value() };

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

	CHECK_WITH_LOG(vkCreateDevice(gpu, &create_info, nullptr, &(device)) != VK_SUCCESS,"RHI Error: failed to create logical device!");
	CreateQueue(queue_family_indices);
}



void VK_Device::GetDeviceExtensionsAndLayers(VkPhysicalDevice Gpu, UInt32 VendorId, Vector<CONST Char*>& OutDeviceExtensions, Vector<CONST Char*>& OutDeviceLayers, Vector<String>& OutAllDeviceExtensions, Vector<String>& OutAllDeviceLayers, Bool& bOutDebugMarkers)
{


}
void VK_Device::CreateQueue(QueueFamilyIndices family_indice)
{
	graph_queue=new VK_Queue(this,family_indice.graphics_family.value());
	compute_queue = new VK_Queue(this, family_indice.compute_family.value());
	//present_queue = new VK_Queue(this, family_indice.presentFamily.value());
}

void VK_Device::Init(Int device_index,Bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers)
{
	CreateDevice(enable_validation_layers, std::move(device_extensions), std::move(validation_layers));

	device_memory_manager=new VK_DeviceMemoryManager(this);
	memory_manager=new VK_MemoryManager(this);
	fence_manager=new VK_FenceManager(this);
	command_buffer_manager=new VK_CommandBufferManager(this);
}

VkDevice VK_Device::GetDevice()
{
	return device;
}

VkPhysicalDevice VK_Device::GetGpu()
{
	return gpu;
}

VK_FenceManager* VK_Device::GetFenceManager()
{
	return fence_manager;
}

VK_DeviceMemoryManager* VK_Device::GetDeviceMemoryManager()
{
	return device_memory_manager;
}

VK_MemoryManager* VK_Device::GetMemoryManager()
{
	return memory_manager;
}

CONST OptionalVulkanDeviceExtensions& VK_Device::GetOptionalExtensions() CONST
{
	return extensions;
}

void VK_Device::CreatePresentQueue(VkSurfaceKHR surface)
{
	if(!present_queue)
	{
		auto check_and_create_queue=[&](VK_Queue* queue) ->Bool
		{
			VkBool32 present_support = false;
			if(queue)
			{
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue->GetFamily(), surface, &present_support);
				if (present_support&& !present_queue) {
					present_queue = new VK_Queue(this, graph_queue->GetFamily());
				}
			}
			return present_support;
		};

		CHECK_WITH_LOG(check_and_create_queue(graph_queue) || check_and_create_queue(compute_queue)||check_and_create_queue(transfer_queue),
						"RHI Error: failed to create present queue!")
	}
}

CONST VkPhysicalDeviceLimits& VK_Device::GetLimits() CONST
{
	return gpu_props.limits;
}

QueueFamilyIndices VK_Device::GetQueueFamilyIndices() CONST
{
	return queue_family_indices;
}

VK_Device::~VK_Device()
{
	if(device)
	{ 
		Destroy();
		device=VK_NULL_HANDLE;
	}
}

VK_Device::VK_Device(VulkanRHI* in_vulkan_rhi, VkPhysicalDevice in_gpu):
	gpu(in_gpu),
	vulkan_rhi(in_vulkan_rhi)
{
	vkGetPhysicalDeviceProperties(gpu,&gpu_props);
	vendor_id=gpu_props.vendorID;
}

void VK_Device::Destroy()
{
	if (graph_queue)
	{
		delete graph_queue;
		graph_queue=nullptr;
	}
	if (compute_queue)
	{
		delete compute_queue;
		compute_queue=nullptr;
	}
	if (transfer_queue)
	{
		delete transfer_queue;
		transfer_queue=nullptr;
	}
	if (present_queue)
	{
		delete present_queue;
		present_queue=nullptr;
	}
	if (fence_manager)
	{
		delete fence_manager;
		fence_manager=nullptr;
	}
	if (device_memory_manager)
	{
		delete device_memory_manager;
		device_memory_manager=nullptr;
	}
	if (memory_manager)
	{
		delete memory_manager;
		memory_manager=nullptr;
	}
	if (command_buffer_manager)
	{
		delete command_buffer_manager;
		command_buffer_manager=nullptr;
	}

	if (device)
	{
		vkDestroyDevice(device, VULKAN_CPU_ALLOCATOR);
		device=VK_NULL_HANDLE;
	}	
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE