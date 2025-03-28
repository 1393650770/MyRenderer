#pragma once
#ifndef _VK_EXTENSION_
#define _VK_EXTENSION_

#include "Core/ConstDefine.h"
#include "Vk_Device.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

#define VULKAN_EXTENSION_NOT_PROMOTED UINT32_MAX
#define VULKAN_EXTENSION_ENABLED      1
#define VULKAN_EXTENSION_DISABLED     0

class VK_Device;
struct OptionalVulkanDeviceExtensions;

MYRENDERER_BEGIN_CLASS(VK_Extension)
#pragma region METHOD
public:
	enum class ENUM_ExtensionActivation :UInt8
	{
		AutoActivate,
		ManullyActivate
	};
	VK_Extension(VK_Device* in_device,CONST String in_extension_name,Int in_enable_in_code ,UInt32 in_promoted_version , UniquePtr<std::function<void(OptionalVulkanDeviceExtensions& extion_flags)>>&& in_flag_setter=nullptr, ENUM_ExtensionActivation in_extension_activate=ENUM_ExtensionActivation::AutoActivate);
	VIRTUAL ~VK_Extension();

	static Vector<UniquePtr<VK_Extension>> METHOD(GetRenderSupportGpuFeatures)(VK_Device* in_device, UInt32 api_version);

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) {}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() {}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) {}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(struct OptionalVulkanDeviceExtensions* extension_flags) {}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) {}

	struct VK_DeviceFeatureProperties& METHOD(GetDeviceExtensionProperties)();
	static Vector<VkExtensionProperties> GetDriverSupportedDeviceExtensions(VkPhysicalDevice gpu, CONST Char* layer_name = nullptr);
	template <typename ExtensionType>
	static Int FindExtension(CONST Vector<UniquePtr<ExtensionType>>& support_extensions, CONST Char* extension_name)
	{
		for (Int index = 0; index < support_extensions.size(); ++index)
		{
			if (support_extensions[index]->GetName()== extension_name)
			{
				return index;
			}
		}
		return -1;
	}
	Bool METHOD(GetIsInUse)() CONST;
	CONST String& METHOD(GetName)() CONST;
	void METHOD(SetIsSupported)();
	void METHOD(SetActivated)();
	Bool METHOD(SetIsCore)(UInt32 api_version);

protected:
	UniquePtr<std::function<void(OptionalVulkanDeviceExtensions& extion_flags)>> flag_setter;
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	union
	{
		struct
		{
			UInt8 is_enable_incode : 1;
			UInt8 is_supported : 1;
			UInt8 is_activate : 1;
			UInt8 is_core : 1;
			UInt8 is_requirements_passed :1;
		};
		UInt8 packed;
	};

	VK_Device* device;
	String name;
	UInt32 promoted_version=UInt32(-1);

private:

#pragma endregion
MYRENDERER_END_CLASS


#define DEVICE_EXT_FLAG_SETTER(FLAG_NAME) std::make_unique<std::function<void(MXRender::RHI::Vulkan::OptionalVulkanDeviceExtensions& extion_flags)>>(std::move([](MXRender::RHI::Vulkan::OptionalVulkanDeviceExtensions& ExtensionFlags) { ExtensionFlags.FLAG_NAME = 1; }))

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
