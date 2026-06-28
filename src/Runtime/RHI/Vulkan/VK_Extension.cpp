#include "VK_Extension.h"
#include "vulkan/vulkan_core.h"
#include "Vk_Device.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

template <typename ExistingChainType, typename NewStructType>
static void AddToPNext(ExistingChainType& existing, NewStructType& added)
{
	added.pNext = (void*)existing.pNext;
	existing.pNext = (void*)&added;
}

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_BufferDeviceAddressExtension, public VK_Extension)
public: 
	VK_BufferDeviceAddressExtension(VK_Device* in_device) 
	: VK_Extension(in_device, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}
	
	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{
	}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		is_requirements_passed = (actual_vk_khr.bufferDeviceAddress == VK_TRUE);
		extension_flags->HasBufferDeviceAddress = is_requirements_passed;
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL 
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR actual_vk_khr;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_AccelerationStructureExtension, public VK_Extension)
public:
	VK_AccelerationStructureExtension(VK_Device* in_device)
		: VK_Extension(in_device, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{
		VkPhysicalDeviceAccelerationStructurePropertiesKHR& acceleration_structure = GetDeviceExtensionProperties().AccelerationStructureProps;
		acceleration_structure.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		AddToPNext(physical_device_properties2, acceleration_structure);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{

	}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		is_requirements_passed = (actual_vk_khr.accelerationStructure == VK_TRUE && actual_vk_khr.descriptorBindingAccelerationStructureUpdateAfterBind == VK_TRUE);
		extension_flags->HasAccelerationStructure = is_requirements_passed;
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceAccelerationStructureFeaturesKHR actual_vk_khr;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_RayTracingPipelineExtension, public VK_Extension)
public:
	VK_RayTracingPipelineExtension(VK_Device* in_device)
		: VK_Extension(in_device, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR& properties_khr = GetDeviceExtensionProperties().RayTracingPipelineProps;
		properties_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		AddToPNext(physical_device_properties2, properties_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		is_requirements_passed = (actual_vk_khr.rayTracingPipeline == VK_TRUE && actual_vk_khr.rayTraversalPrimitiveCulling == VK_TRUE);
		extension_flags->HasRayTracingPipeline = is_requirements_passed;
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR actual_vk_khr;

MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_RayQueryExtension, public VK_Extension)
public:
	VK_RayQueryExtension(VK_Device* in_device)
		: VK_Extension(in_device, VK_KHR_RAY_QUERY_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		is_requirements_passed = (actual_vk_khr.rayQuery == VK_TRUE );
		extension_flags->HasRayQuery = is_requirements_passed;
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceRayQueryFeaturesKHR actual_vk_khr;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_RayTracingPositionFetchExtension, public VK_Extension)
public:
	VK_RayTracingPositionFetchExtension(VK_Device* in_device)
		: VK_Extension(in_device, VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{}
	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		is_requirements_passed = (actual_vk_khr.rayTracingPositionFetch == VK_TRUE);
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR actual_vk_khr;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_DescriptorIndexingExtension, public VK_Extension)
public:
	VK_DescriptorIndexingExtension(VK_Device* in_device)
		: VK_Extension(in_device, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2) {}

	VIRTUAL void METHOD(PrePhysicalDeviceProperties)(VkPhysicalDeviceProperties2KHR& physical_device_properties2) OVERRIDE FINAL
	{
		VkPhysicalDeviceDescriptorIndexingProperties& props = GetDeviceExtensionProperties().DescriptorIndexingProps;
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
		props.pNext = nullptr;
		AddToPNext(physical_device_properties2, props);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceProperties)() OVERRIDE FINAL
	{
		// Store maxUpdateAfterBindDescriptors for later use
		const auto& props = GetDeviceExtensionProperties().DescriptorIndexingProps;
		max_update_after_bind_sampled_images = props.maxDescriptorSetUpdateAfterBindSampledImages;
		max_update_after_bind_samplers = props.maxDescriptorSetUpdateAfterBindSamplers;
		max_per_stage_update_after_bind_sampled_images = props.maxPerStageDescriptorUpdateAfterBindSampledImages;
	}

	VIRTUAL void METHOD(PrePhysicalDeviceFeatures)(VkPhysicalDeviceFeatures2KHR& physical_device_features2) OVERRIDE FINAL
	{
		actual_vk_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		actual_vk_khr.pNext = nullptr;
		AddToPNext(physical_device_features2, actual_vk_khr);
	}
	VIRTUAL void METHOD(PostPhysicalDeviceFeatures)(OptionalVulkanDeviceExtensions* extension_flags) OVERRIDE FINAL
	{
		// Require the minimal set of features needed for bindless
		is_requirements_passed = (
			actual_vk_khr.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE &&
			actual_vk_khr.descriptorBindingPartiallyBound == VK_TRUE &&
			actual_vk_khr.runtimeDescriptorArray == VK_TRUE &&
			actual_vk_khr.shaderSampledImageArrayNonUniformIndexing == VK_TRUE &&
			actual_vk_khr.descriptorBindingVariableDescriptorCount == VK_TRUE
		);
		if (is_requirements_passed)
		{
			extension_flags->HasEXTDescriptorIndexing = 1;
		}
	}
	VIRTUAL void METHOD(PreCreateDevice)(VkDeviceCreateInfo& device_info) OVERRIDE FINAL
	{
		if (is_requirements_passed)
		{
			AddToPNext(device_info, actual_vk_khr);
		}
	}
	VkPhysicalDeviceDescriptorIndexingFeatures actual_vk_khr;

	// Cached limits for bindless
	UInt32 max_update_after_bind_sampled_images = 0;
	UInt32 max_update_after_bind_samplers = 0;
	UInt32 max_per_stage_update_after_bind_sampled_images = 0;

MYRENDERER_END_CLASS

VK_Extension::VK_Extension(VK_Device* in_device, CONST String in_extension_name, Int in_enable_in_code, UInt32 in_promoted_version, UniquePtr<std::function<void(MXRender::RHI::Vulkan::OptionalVulkanDeviceExtensions& extion_flags)>>&& in_flag_setter, ENUM_ExtensionActivation in_extension_activate)
	:device(in_device),name(in_extension_name),is_enable_incode(in_enable_in_code),is_supported(false),is_core(false),is_activate(in_extension_activate==ENUM_ExtensionActivation::AutoActivate), promoted_version(in_promoted_version),flag_setter(std::move(in_flag_setter))
{

}

VK_Extension::~VK_Extension()
{

}

template <typename ExtensionType>
static void FlagExtensionSupport(CONST Vector<VkExtensionProperties>& extension_properties, Vector<UniquePtr<ExtensionType>>& extensions, UInt32 api_version, CONST Char* extension_type_name)
{

	for (CONST VkExtensionProperties& Extension : extension_properties)
	{
		CONST Int extension_index = ExtensionType::FindExtension(extensions, Extension.extensionName);
		CONST Bool bFound = (extension_index != -1);
		if (bFound)
		{
			extensions[extension_index]->SetIsSupported();
			extensions[extension_index]->SetIsCore(api_version);
		}
	}
}



Vector<UniquePtr<VK_Extension>> VK_Extension::GetRenderSupportGpuFeatures(VK_Device* in_device, UInt32 api_version)
{
	Vector<UniquePtr<VK_Extension>> return_device_extensions;
	#define ADD_CUSTOM_EXTENSION(EXTENSION_CLASS) \
		return_device_extensions.emplace_back(std::make_unique<EXTENSION_CLASS>(in_device));
	
	#define ADD_SIMPLE_EXTENSION(EXTENSION_NAME,ENABLE_IN_CODE,PROMOTED_VER,FLAG_SETTER)\
	    return_device_extensions.emplace_back(std::make_unique<VK_Extension>(in_device,EXTENSION_NAME,ENABLE_IN_CODE,PROMOTED_VER,FLAG_SETTER));
	
	/*	ADD_SIMPLE_EXTENSION(VK_KHR_SWAPCHAIN_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VULKAN_EXTENSION_NOT_PROMOTED, nullptr);
		ADD_SIMPLE_EXTENSION(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VULKAN_EXTENSION_NOT_PROMOTED, DEVICE_EXT_FLAG_SETTER(HasMemoryBudget));
		ADD_SIMPLE_EXTENSION(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VULKAN_EXTENSION_NOT_PROMOTED, DEVICE_EXT_FLAG_SETTER(HasMemoryPriority));
		ADD_SIMPLE_EXTENSION(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2, DEVICE_EXT_FLAG_SETTER(HasKHRRenderPass2));
		ADD_SIMPLE_EXTENSION(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VULKAN_EXTENSION_NOT_PROMOTED, DEVICE_EXT_FLAG_SETTER(HasDeferredHostOperations));
		*/
	ADD_SIMPLE_EXTENSION(VK_KHR_SPIRV_1_4_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2, DEVICE_EXT_FLAG_SETTER(HasSPIRV_14));
	ADD_SIMPLE_EXTENSION(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2, DEVICE_EXT_FLAG_SETTER(HasShaderFloatControls));
	//ADD_SIMPLE_EXTENSION(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VK_API_VERSION_1_2, DEVICE_EXT_FLAG_SETTER(HasKHRImageFormatList));
	//ADD_SIMPLE_EXTENSION(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME, VULKAN_EXTENSION_ENABLED, VULKAN_EXTENSION_NOT_PROMOTED, DEVICE_EXT_FLAG_SETTER(HasEXTValidationCache));

	ADD_CUSTOM_EXTENSION(VK_BufferDeviceAddressExtension);
	ADD_CUSTOM_EXTENSION(VK_AccelerationStructureExtension);
	ADD_CUSTOM_EXTENSION(VK_RayTracingPipelineExtension);
	ADD_CUSTOM_EXTENSION(VK_RayQueryExtension);
	ADD_CUSTOM_EXTENSION(VK_RayTracingPositionFetchExtension);
	ADD_CUSTOM_EXTENSION(VK_DescriptorIndexingExtension);
	FlagExtensionSupport(GetDriverSupportedDeviceExtensions(in_device->GetGpu()), return_device_extensions, api_version, ("device"));
	return std::move(return_device_extensions);
}

VK_DeviceFeatureProperties& VK_Extension::GetDeviceExtensionProperties()
{
	CONST VK_DeviceFeatureProperties& extension_properties = device->GetOptionalExtensionProperties();
	return CONST_CAST(extension_properties,VK_DeviceFeatureProperties&);
}

Bool VK_Extension::GetIsInUse() CONST
{
	return is_enable_incode && is_supported && is_activate;
}

CONST String& VK_Extension::GetName() CONST
{
	return name;
}

void VK_Extension::SetIsSupported()
{
	is_supported = true;
}

Bool VK_Extension::SetIsCore(UInt32 api_version)
{
	return (is_core = (api_version >= promoted_version));
}

Vector<VkExtensionProperties> VK_Extension::GetDriverSupportedDeviceExtensions(VkPhysicalDevice gpu, CONST Char* layer_name /*= nullptr*/)
{
	Vector<VkExtensionProperties> out_device_extensions;
	UInt32 Count = 0;
	vkEnumerateDeviceExtensionProperties(gpu, layer_name, &Count, nullptr);
	if (Count > 0)
	{
		out_device_extensions.resize(Count);
		vkEnumerateDeviceExtensionProperties(gpu, layer_name, &Count, out_device_extensions.data());
	}
	std::sort(out_device_extensions.begin(), out_device_extensions.end(), [](const VkExtensionProperties& A, const VkExtensionProperties& B) { return strcmp(A.extensionName, B.extensionName) < 0; });
	return out_device_extensions;
}

void VK_Extension::SetActivated()
{
	is_activate = true;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE