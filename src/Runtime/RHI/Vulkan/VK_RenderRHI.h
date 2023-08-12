#pragma once
#ifndef _VUKANRENDERRHI_
#define _VUKANRENDERRHI_
#include "../../Core/ConstDefine.h"
#include "../RenderRHI.h"
#include "vulkan/vulkan_core.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(VulkanRHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRenderFactory,public RenderFactory)
public:
	
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRHI,RenderRHI)

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

void METHOD(CreateInstance)(Bool enable_validation_layers);
void METHOD(InitializeDebugmessenger)(Bool enable_validation_layers);
void METHOD(CreateSurface)();

Bool METHOD(CheckValidationlayerSupport)();
Vector<CONST Char*> METHOD(GetRequiredExtensions)(Bool enable_validation_layers);
void METHOD(PopulateDebugMessengerCreateInfo)(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
VkResult METHOD(create_debug_utils_messengerEXT)(VkInstance instance, CONST VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, CONST VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
protected:

#pragma endregion

#pragma region MEMBER
public:
private:
protected:
	VkInstance instance=VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR surface;
#pragma endregion

MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

