#pragma once
#ifndef _VK_FRAMEBUFFER_
#define _VK_FRAMEBUFFER_
#include "RHI/RenderFrameBuffer.h"
#include "vulkan/vulkan_core.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

class VK_Device;
class VK_RenderPass;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(VK_FrameBuffer,public FrameBuffer)

#pragma region METHOD
    public:
        VK_FrameBuffer(VK_Device* in_device, CONST FrameBufferDesc& in_desc, VkRenderPass in_render_pass);
        VIRTUAL ~VK_FrameBuffer() ;
		VkFramebuffer METHOD(GetFramebuffer)() CONST;
    protected:

    private:
#pragma endregion

#pragma region MEMBER
	public:

	protected:
        VK_Device* device;  
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
	private:
#pragma endregion
        
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_STRUCT(FramebufferCacheKey)
public:
	VkRenderPass render_pass = VK_NULL_HANDLE;
	Vector<Texture*> render_targets;
	Texture* depth_stencil;
	VkImageView  shading_rate = VK_NULL_HANDLE;
	UInt64       command_queue_mask = 0;

	Bool   operator==(CONST FramebufferCacheKey& rhs) CONST;
	UInt64 GetHash() CONST;

private:
	mutable UInt64 hash = 0;
MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_STRUCT(FramebufferCacheKeyHash)
public:
	std::size_t operator()(CONST FramebufferCacheKey& key) CONST
	{
		return key.GetHash();
	}
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(VK_FrameBufferManager)

#pragma region METHOD
	public:
		VK_FrameBufferManager(VK_Device* in_device);
		VK_FrameBufferManager(CONST VK_FrameBufferManager&) MYDELETE;
		VK_FrameBufferManager(VK_FrameBufferManager&&) MYDELETE;
		VK_FrameBufferManager& operator=(CONST VK_FrameBufferManager&) MYDELETE;
		VK_FrameBufferManager& operator=(VK_FrameBufferManager&&) MYDELETE;
		VIRTUAL ~VK_FrameBufferManager();

		VK_FrameBuffer* METHOD(GetFramebuffer)(CONST FramebufferCacheKey& key, uint32_t width, uint32_t height, uint32_t layers);
		void          METHOD(OnDestroyImageView)(VkImageView image_view);
		void          METHOD(OnDestroyRenderPass)(VkRenderPass render_pass);
	protected:
		void METHOD(Destroy)();
	private:
#pragma endregion

#pragma region MEMBER
	public:

	protected:
		VK_Device* device;
		Map<FramebufferCacheKey, VK_FrameBuffer*, FramebufferCacheKeyHash> framebuffer_cache;

		Map<VkImageView, Vector<FramebufferCacheKey>>  view_to_key_map;
		Map<VkRenderPass, Vector<FramebufferCacheKey>> render_pass_to_key_map;
	private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE


#endif
