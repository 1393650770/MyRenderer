#pragma once

#ifndef _VK_DEVICE_
#define _VK_DEVICE_
#include <vulkan/vulkan.h>
#include "../../Core/ConstDefine.h"
#include "../RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

MYRENDERER_BEGIN_CLASS(OptionalVulkanDeviceExtensions)
union
{
	struct
	{
		UINT32 HasKHRMaintenance1 : 1;
		UINT32 HasKHRMaintenance2 : 1;
		//uint32 HasMirrorClampToEdge : 1;
		UINT32 HasKHRDedicatedAllocation : 1;
		UINT32 HasEXTValidationCache : 1;
		UINT32 HasAMDBufferMarker : 1;
		UINT32 HasNVDiagnosticCheckpoints : 1;
		UINT32 HasNVDeviceDiagnosticConfig : 1;
		UINT32 HasYcbcrSampler : 1;
		UINT32 HasMemoryPriority : 1;
		UINT32 HasMemoryBudget : 1;
		UINT32 HasDriverProperties : 1;
		UINT32 HasEXTFragmentDensityMap : 1;
		UINT32 HasEXTFragmentDensityMap2 : 1;
		UINT32 HasKHRFragmentShadingRate : 1;
		UINT32 HasEXTFullscreenExclusive : 1;
		UINT32 HasKHRImageFormatList : 1;
		UINT32 HasEXTASTCDecodeMode : 1;
		UINT32 HasQcomRenderPassTransform : 1;
		UINT32 HasBufferAtomicInt64 : 1;
		UINT32 HasScalarBlockLayoutFeatures : 1;
		UINT32 HasKHRMultiview : 1;
	};
	UINT32 Packed;
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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_Device , public RenderResource)
#pragma region METHOD

private:
protected:
public:
	VK_Device();
	VIRTUAL ~VK_Device();

	void METHOD(Init)(int device_index );
#pragma endregion

#pragma region MEMBER

private:
protected:
public:
	VkDevice device;
	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties gpu_props;

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif //
