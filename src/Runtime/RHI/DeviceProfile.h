#pragma once
#ifndef _DEVICE_PROFILE_
#define _DEVICE_PROFILE_

#include "Core/ConstDefine.h"
#include <vulkan/vulkan_core.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
class VK_Device;
MYRENDERER_END_NAMESPACE

// Summarised device capability report extracted from VK_Device.
// Used by ShaderLibrary and upper layers to choose code paths
// (shader SPIR-V version, bindless vs traditional descriptors,
// workgroup size limits, compressed texture format availability).
MYRENDERER_BEGIN_STRUCT(DeviceCapability)
public:
	// Vulkan API version reported by the device (VK_API_VERSION_1_0 .. 1_3)
	UInt32 api_version = VK_API_VERSION_1_0;

	// Compute limits
	UInt32 max_compute_workgroup_invocations = 128;

	// Feature toggles
	Bool supports_spirv_14 = false;           // #version 460 / SPIR-V 1.4+
	Bool supports_bindless = false;           // VK_EXT_descriptor_indexing with all sub-features
	Bool supports_dynamic_rendering = false;  // VK_KHR_dynamic_rendering (1.3 core)
	Bool supports_buffer_device_address = false;

	// Memory architecture
	Bool   is_unified_memory = false;         // mobile / integrated GPU
	Bool   is_desktop_gpu = false;            // discrete GPU
	UInt64 dedicated_vram_mb = 0;

	// Compressed texture formats
	Bool supports_astc = false;
	Bool supports_etc2 = true;                // mandatory in Vulkan Android baseline
	Bool supports_bc = false;                 // desktop BCn

	// Hardware vendor
	UInt32 vendor_id = 0;

	// Populate from a fully-initialised VK_Device.
	// Must be called after VK_Device::Init().
	static DeviceCapability METHOD(Query)(Vulkan::VK_Device* in_device);
MYRENDERER_END_STRUCT

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // _DEVICE_PROFILE_
