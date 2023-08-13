#pragma once

#ifndef _VK_DEVICE_
#define _VK_DEVICE_
#include <vulkan/vulkan.h>
#include "../../Core/ConstDefine.h"
#include "../RenderRource.h"
#include "VK_RenderRHI.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Queue;

MYRENDERER_BEGIN_CLASS(OptionalVulkanDeviceExtensions)
union
{
	struct
	{
		UInt32 HasKHRMaintenance1 : 1;
		UInt32 HasKHRMaintenance2 : 1;
		UInt32 HasMirrorClampToEdge : 1;
		UInt32 HasKHRDedicatedAllocation : 1;
		UInt32 HasEXTValidationCache : 1;
		UInt32 HasAMDBufferMarker : 1;
		UInt32 HasNVDiagnosticCheckpoints : 1;
		UInt32 HasNVDeviceDiagnosticConfig : 1;
		UInt32 HasYcbcrSampler : 1;
		UInt32 HasMemoryPriority : 1;
		UInt32 HasMemoryBudget : 1;
		UInt32 HasDriverProperties : 1;
		UInt32 HasEXTFragmentDensityMap : 1;
		UInt32 HasEXTFragmentDensityMap2 : 1;
		UInt32 HasKHRFragmentShadingRate : 1;
		UInt32 HasEXTFullscreenExclusive : 1;
		UInt32 HasKHRImageFormatList : 1;
		UInt32 HasEXTASTCDecodeMode : 1;
		UInt32 HasQcomRenderPassTransform : 1;
		UInt32 HasBufferAtomicInt64 : 1;
		UInt32 HasScalarBlockLayoutFeatures : 1;
		UInt32 HasKHRMultiview : 1;
	};
	UInt32 Packed;
};

OptionalVulkanDeviceExtensions()
{
	static_assert(sizeof(Packed) == sizeof(OptionalVulkanDeviceExtensions), "More bits needed for Packed!");
	Packed = 0;
}

inline bool METHOD(HasGPUCrashDumpExtensions)() const
{
	return HasAMDBufferMarker || HasNVDiagnosticCheckpoints;
}
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS(QueueFamilyIndices)
public:
std::optional<uint32_t> graphicsFamily;
//std::optional<uint32_t> presentFamily;
std::optional<uint32_t> transferFamily;
std::optional<uint32_t> computeFamily;
bool isComplete() {
	return graphicsFamily.has_value() &&  computeFamily.has_value();
}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Device , public RenderResource)

#pragma region METHOD

private:
	QueueFamilyIndices METHOD(FindQueueFamilies)(VkPhysicalDevice device);
protected:
	void METHOD(CreateDevice)(bool enableValidationLayers, Vector<CONST Char*> deviceExtensions, Vector<CONST Char*> validationLayers);
	static void METHOD(GetDeviceExtensionsAndLayers)(VkPhysicalDevice Gpu, UInt32 VendorId, Vector<CONST Char*>& OutDeviceExtensions, Vector<CONST Char*>& OutDeviceLayers, Vector<String>& OutAllDeviceExtensions, Vector<String>& OutAllDeviceLayers, bool& bOutDebugMarkers);
	void METHOD(CreateQueue)(QueueFamilyIndices family_indice);
public:
	VK_Device(VulkanRHI* in_vulkan_rhi, VkPhysicalDevice in_gpu);
	VIRTUAL ~VK_Device();

void METHOD(Init)(int device_index, bool enableValidationLayers, Vector<CONST Char*> deviceExtensions, Vector<CONST Char*> validationLayers);

	VkDevice METHOD(GetDevice)();
	VkPhysicalDevice METHOD(GetGpu)();

#pragma endregion

#pragma region MEMBER
	
private:
protected:
	VkDevice device;
	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties gpu_props;
	UInt32 vendor_id;
	VulkanRHI* vulkan_rhi;

	VK_Queue* graph_queue;
	VK_Queue* compute_queue;
	VK_Queue* transfer_queue;
	VK_Queue* present_queue;
public:


#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //
