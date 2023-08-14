#pragma once
#ifndef _VUKANRENDERRHI_
#define _VUKANRENDERRHI_
#include "../../Core/ConstDefine.h"
#include "../RenderRHI.h"
#include "vulkan/vulkan_core.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRenderFactory,public RenderFactory)
public:
	
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRHI,public RenderRHI)

#pragma region MATHOD

public:
	VulkanRHI() DEFAULT;
	VIRTUAL ~VulkanRHI() DEFAULT;

#pragma region INIT_MATHOD

	VIRTUAL void METHOD(Init)(RenderFactory* render_factory) OVERIDE FINAL;


	VIRTUAL void METHOD(PostInit)() OVERIDE FINAL;


	VIRTUAL void METHOD(Shutdown)() OVERIDE FINAL;
#pragma endregion


#pragma region CREATE_RESOURCE
	

#pragma endregion


private:
Bool METHOD(CheckGpuSuitable)(VkPhysicalDevice gpu);

VkPhysicalDevice METHOD(GetGpuFromHarddrive)();
void METHOD(CreateDevice)(Bool enable_validation_layers);
void METHOD(CreateInstance)(Bool enable_validation_layers);
void METHOD(InitializeDebugmessenger)(Bool enable_validation_layers);

Bool METHOD(CheckValidationlayerSupport)();
Vector<CONST Char*> METHOD(GetRequiredExtensions)(Bool enable_validation_layers);
void METHOD(PopulateDebugMessengerCreateInfo)(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
VkResult METHOD(CreateDebugUtilsMessengerEXT)(VkInstance instance, CONST VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, CONST VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
protected:

#pragma endregion

#pragma region MEMBER
public:
private:
protected:
	VkInstance instance=VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger;
	VK_Device* device=nullptr;
	Vector<VK_Viewport*> viewports;

	friend class VK_Viewport;
#pragma endregion

MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

