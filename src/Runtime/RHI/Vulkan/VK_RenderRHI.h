#pragma once
#ifndef _VUKANRENDERRHI_
#define _VUKANRENDERRHI_
#include "RHI/RenderRHI.h"
#include "vulkan/vulkan_core.h"
#include "VK_Viewport.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
class Shader;
class Buffer;

MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;
class VK_Viewport;
class VK_CommandBuffer;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRenderFactory,public RenderFactory)
public:
	
MYRENDERER_END_CLASS


//RHI is mostly responsible for resource creation
//Context is responsible for rendering command commit records
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VulkanRHI,public RenderRHI)

#pragma region MATHOD

public:
	VulkanRHI() MYDEFAULT;
	VIRTUAL ~VulkanRHI() MYDEFAULT;

#pragma region INIT_MATHOD

	VIRTUAL void METHOD(Init)(RenderFactory* render_factory) OVERRIDE FINAL;


	VIRTUAL void METHOD(PostInit)() OVERRIDE FINAL;


	VIRTUAL void METHOD(Shutdown)() OVERRIDE FINAL;

#pragma endregion


#pragma region CREATE_RESOURCE
	VIRTUAL Viewport* METHOD(CreateViewport)(void* window_handle, Int width, Int height, Bool is_full_screen) OVERRIDE FINAL;
	VIRTUAL Shader* CreateShader(CONST ShaderDesc& desc, CONST ShaderDataPayload& data)  OVERRIDE FINAL;
	VIRTUAL Buffer* METHOD(CreateBuffer)(const BufferDesc& buffer_desc) OVERRIDE FINAL;
	VIRTUAL Texture* METHOD(CreateTexture)(CONST TextureDesc& texture_desc) OVERRIDE FINAL;
	VIRTUAL RenderPipelineState* METHOD(CreateRenderPipelineState)(CONST RenderGraphiPipelineStateDesc& desc) OVERRIDE FINAL;
	VIRTUAL RenderPass* METHOD(CreateRenderPass)(CONST RenderPassDesc& desc) OVERRIDE FINAL;
	VIRTUAL FrameBuffer* METHOD(CreateFrameBuffer)(CONST FrameBufferDesc& desc) OVERRIDE FINAL;

	VIRTUAL void* METHOD(MapBuffer)(Buffer* buffer, ENUM_MAP_TYPE map_type, ENUM_MAP_FLAG map_flag) OVERRIDE FINAL;
	VIRTUAL void METHOD(UnmapBuffer)(Buffer* buffer) OVERRIDE FINAL;
#pragma endregion

#pragma region DRAW
	VIRTUAL	CommandList* METHOD(GetImmediateCommandList)() OVERRIDE FINAL;
	VIRTUAL void METHOD(SubmitCommandList)(CommandList* command_list) OVERRIDE FINAL;
	VIRTUAL void METHOD(RenderEnd)() OVERRIDE FINAL;
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
	VK_CommandBuffer* immediate_command_buffer=nullptr;
	Vector<VK_CommandBuffer*> defered_command_buffers;
	friend class VK_Viewport;
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

