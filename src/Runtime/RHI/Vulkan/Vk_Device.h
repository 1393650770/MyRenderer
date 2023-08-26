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
class VK_FenceManager;
class VK_DeviceMemoryManager;
MYRENDERER_BEGIN_STRUCT(OptionalVulkanDeviceExtensions)
union
{
	struct
	{
		// Optional Extensions
		UInt64 HasEXTValidationCache : 1;
		UInt64 HasMemoryPriority : 1;
		UInt64 HasMemoryBudget : 1;
		UInt64 HasEXTASTCDecodeMode : 1;
		UInt64 HasEXTFragmentDensityMap : 1;
		UInt64 HasEXTFragmentDensityMap2 : 1;
		UInt64 HasKHRFragmentShadingRate : 1;
		UInt64 HasEXTFullscreenExclusive : 1;
		UInt64 HasImageAtomicInt64 : 1;
		UInt64 HasAccelerationStructure : 1;
		UInt64 HasRayTracingPipeline : 1;
		UInt64 HasRayQuery : 1;
		UInt64 HasDeferredHostOperations : 1;
		UInt64 HasEXTCalibratedTimestamps : 1;
		UInt64 HasEXTDescriptorBuffer : 1;
		UInt64 HasEXTDeviceFault : 1;

		// Vendor specific
		UInt64 HasAMDBufferMarker : 1;
		UInt64 HasNVDiagnosticCheckpoints : 1;
		UInt64 HasNVDeviceDiagnosticConfig : 1;
		UInt64 HasQcomRenderPassTransform : 1;

		// Promoted to 1.1
		UInt64 HasKHRMultiview : 1;
		UInt64 HasKHR16bitStorage : 1;

		// Promoted to 1.2
		UInt64 HasKHRRenderPass2 : 1;
		UInt64 HasKHRImageFormatList : 1;
		UInt64 HasKHRShaderAtomicInt64 : 1;
		UInt64 HasEXTScalarBlockLayout : 1;
		UInt64 HasBufferDeviceAddress : 1;
		UInt64 HasSPIRV_14 : 1;
		UInt64 HasShaderFloatControls : 1;
		UInt64 HasKHRShaderFloat16 : 1;
		UInt64 HasEXTDescriptorIndexing : 1;
		UInt64 HasEXTShaderViewportIndexLayer : 1;
		UInt64 HasSeparateDepthStencilLayouts : 1;
		UInt64 HasEXTHostQueryReset : 1;

		// Promoted to 1.3
		UInt64 HasEXTTextureCompressionASTCHDR : 1;
		UInt64 HasKHRMaintenance4 : 1;
		UInt64 HasKHRSynchronization2 : 1;
		UInt64 HasEXTSubgroupSizeControl : 1;
		
	};
	UInt64 Packed;
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
MYRENDERER_END_STRUCT


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
	void METHOD(CreateQueue)(QueueFamilyIndices family_indice);
protected:
	void METHOD(CreateDevice)(bool enableValidationLayers, Vector<CONST Char*> deviceExtensions, Vector<CONST Char*> validationLayers);
	static void METHOD(GetDeviceExtensionsAndLayers)(VkPhysicalDevice Gpu, UInt32 VendorId, Vector<CONST Char*>& OutDeviceExtensions, Vector<CONST Char*>& OutDeviceLayers, Vector<String>& OutAllDeviceExtensions, Vector<String>& OutAllDeviceLayers, bool& bOutDebugMarkers);
	
public:
	VK_Device(VulkanRHI* in_vulkan_rhi, VkPhysicalDevice in_gpu);
	VIRTUAL ~VK_Device();

	void METHOD(Init)(int device_index, bool enableValidationLayers, Vector<CONST Char*> deviceExtensions, Vector<CONST Char*> validationLayers);

	VkDevice METHOD(GetDevice)();
	VkPhysicalDevice METHOD(GetGpu)();
	VK_FenceManager* METHOD(GetFenceManager)();
	VK_DeviceMemoryManager* METHOD(GetDeviceMemoryManager)();
	CONST OptionalVulkanDeviceExtensions& METHOD(GetOptionalExtensions)() CONST;
	void METHOD(CreatePresentQueue)(VkSurfaceKHR surface);
	CONST VkPhysicalDeviceLimits&  METHOD(GetLimits)() CONST;
#pragma endregion

#pragma region MEMBER
	
private:
protected:
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties gpu_props{};
	UInt32 vendor_id;
	VulkanRHI* vulkan_rhi;
	OptionalVulkanDeviceExtensions extensions;
	VK_Queue* graph_queue = nullptr;
	VK_Queue* compute_queue = nullptr;
	VK_Queue* transfer_queue = nullptr;
	VK_Queue* present_queue = nullptr;

	VK_FenceManager* fence_manager= nullptr;
	VK_DeviceMemoryManager* device_memory_manager=nullptr;
public:


#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //
