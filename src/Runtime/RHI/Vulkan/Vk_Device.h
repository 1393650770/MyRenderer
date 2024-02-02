#pragma once

#ifndef _VK_DEVICE_
#define _VK_DEVICE_
#include <vulkan/vulkan.h>
#include "Core/ConstDefine.h"
#include "RHI/RenderRource.h"
#include "VK_RenderRHI.h"
#include "VK_Memory.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Queue;
class VK_FenceManager;
class VK_DeviceMemoryManager;
class VK_MemoryManager;
class VK_CommandBufferManager;
class VK_StagingBufferManager;
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

inline Bool METHOD(HasGPUCrashDumpExtensions)() CONST
{
	return HasAMDBufferMarker || HasNVDiagnosticCheckpoints;
}
MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_CLASS(QueueFamilyIndices)
public:
std::optional<UInt32> graphics_family;
//std::optional<UInt32> presentFamily;
std::optional<UInt32> transfer_family;
std::optional<UInt32> compute_family;
Bool isComplete() {
	return graphics_family.has_value() &&  compute_family.has_value();
}
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Device , public RenderResource)

#pragma region METHOD

private:
	QueueFamilyIndices METHOD(FindQueueFamilies)(VkPhysicalDevice device);
	void METHOD(CreateQueue)(QueueFamilyIndices family_indice);
protected:
	void METHOD(CreateDevice)(Bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers);
	static void METHOD(GetDeviceExtensionsAndLayers)(VkPhysicalDevice Gpu, UInt32 VendorId, Vector<CONST Char*>& OutDeviceExtensions, Vector<CONST Char*>& OutDeviceLayers, Vector<String>& OutAllDeviceExtensions, Vector<String>& OutAllDeviceLayers, Bool& bOutDebugMarkers);
	void METHOD(Destroy)();
public:
	VK_Device(VulkanRHI* in_vulkan_rhi, VkPhysicalDevice in_gpu);
	VIRTUAL ~VK_Device();

	void METHOD(Init)(Int device_index, Bool enable_validation_layers, Vector<CONST Char*> device_extensions, Vector<CONST Char*> validation_layers);

	VkDevice METHOD(GetDevice)();
	VkPhysicalDevice METHOD(GetGpu)();
	VK_FenceManager* METHOD(GetFenceManager)();
	VK_DeviceMemoryManager* METHOD(GetDeviceMemoryManager)();
	VK_MemoryManager* METHOD(GetMemoryManager)();
	VK_StagingBufferManager* METHOD(GetStagingBufferManager)();
	VK_CommandBufferManager* METHOD(GetCommandBufferManager)();

	VK_Queue* METHOD(GetQueue)(ENUM_QUEUE_TYPE queue_type);
	CONST OptionalVulkanDeviceExtensions& METHOD(GetOptionalExtensions)() CONST;
	void METHOD(CreatePresentQueue)(VkSurfaceKHR surface);
	CONST VkPhysicalDeviceLimits&  METHOD(GetLimits)() CONST;
	QueueFamilyIndices METHOD(GetQueueFamilyIndices)() CONST;
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
	QueueFamilyIndices queue_family_indices;
	VK_FenceManager* fence_manager= nullptr;
	VK_DeviceMemoryManager* device_memory_manager=nullptr;
	VK_MemoryManager* memory_manager = nullptr;
	VK_CommandBufferManager* command_buffer_manager = nullptr;
	VK_StagingBufferManager* staging_buffer_manager = nullptr;
public:


#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //
