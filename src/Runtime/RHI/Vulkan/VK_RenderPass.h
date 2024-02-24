#pragma once
#ifndef _VK_RENDERPASS_
#define _VK_RENDERPASS_
#include "RHI/RenderPass.h"
#include "vulkan/vulkan_core.h"



MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
class VK_Device;
class VK_Shader;
class VK_Viewport;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_RenderPass,public RenderPass)

#pragma region METHOD
public:
	VK_RenderPass(VK_Device* in_device, CONST RenderPassDesc& in_desc);
	VIRTUAL ~VK_RenderPass();
	VkRenderPass METHOD(GetRenderPass)() CONST;
	VIRTUAL void METHOD(Destroy)();
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	VK_Device * device;
	VkRenderPass render_pass;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(VK_RenderPassManager)
public:


#pragma region METHOD
public:
	VK_RenderPassManager(VK_Device* in_device) ;
	VK_RenderPassManager(CONST VK_RenderPassManager&) DELETE;
	VK_RenderPassManager(VK_RenderPassManager&&) DELETE;
	VK_RenderPassManager& operator=(CONST VK_RenderPassManager&) DELETE;
	VK_RenderPassManager& operator=(VK_RenderPassManager&&) DELETE;

	~VK_RenderPassManager();

	VK_RenderPass* METHOD(GetRenderPass)(CONST RenderPassCacheKey& key);
	VK_RenderPass* METHOD(GetRenderPass)(CONST RenderPassDesc& desc);
	void METHOD(Destroy)();
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
	
protected:
	VK_Device* device;
	Map<RenderPassCacheKey, VK_RenderPass*, RenderPassCacheKeyHash> render_pass_cache;
private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
