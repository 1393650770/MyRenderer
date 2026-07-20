#include "DeviceProfile.h"
#include "Vulkan/VK_Device.h"
#include <vulkan/vulkan.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

DeviceCapability DeviceCapability::Query(Vulkan::VK_Device* in_device)
{
	DeviceCapability caps;

	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(in_device->GetGpu(), &props);
	CONST auto& limits = in_device->GetLimits();
	CONST auto& ext = in_device->GetOptionalExtensions();

	caps.api_version = props.apiVersion;
	caps.vendor_id = props.vendorID;
	caps.max_compute_workgroup_invocations = limits.maxComputeWorkGroupInvocations;
	caps.supports_spirv_14 = ext.HasSPIRV_14 != 0;

	// Bindless requires ALL sub-features (enforced by VK_DescriptorIndexingExtension)
	caps.supports_bindless = ext.HasEXTDescriptorIndexing != 0;
	caps.supports_dynamic_rendering = ext.HasKHRDynamicRendering != 0;
	caps.supports_buffer_device_address = ext.HasBufferDeviceAddress != 0;

	// Memory: discrete GPU typically reports DEVICE_LOCAL | HOST_VISIBLE as separate heaps
	caps.is_desktop_gpu = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
	caps.is_unified_memory = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

	// Compressed texture format detection (conservative: true = definitely supported)
	caps.supports_astc = ext.HasEXTTextureCompressionASTCHDR != 0;
	caps.supports_etc2 = true;   // mandatory in Vulkan 1.0 baseline on Android
	caps.supports_bc = (!caps.is_unified_memory && props.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

	return caps;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
