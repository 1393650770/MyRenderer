#pragma once
#ifndef _VK_RENDERPASS_
#define _VK_RENDERPASS_
#include "RHI/RenderPass.h"
#include "Core/TypeHash.h"
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
MYRENDERER_BEGIN_STRUCT(RenderPassCacheKey)
	public:
		RenderPassCacheKey() {}
		RenderPassCacheKey(UInt8 in_num_render_targets, CONST ENUM_TEXTURE_FORMAT* in_render_target_formats, ENUM_TEXTURE_FORMAT in_depth_stencil_format, UInt8 in_sample_count, Bool in_is_enable_vrs, Bool in_is_read_only_dsv);

		Bool operator == (CONST RenderPassCacheKey& rhs) CONST
		{
			return GetHash() == rhs.GetHash() &&
				num_render_targets == rhs.num_render_targets &&
				depth_stencil_format == rhs.depth_stencil_format &&
				sample_count == rhs.sample_count &&
				is_enable_vrs == rhs.is_enable_vrs &&
				is_read_only_dsv == rhs.is_read_only_dsv &&
				memcmp(render_target_formats, rhs.render_target_formats, sizeof(render_target_formats)) == 0;
		}
		UInt64 METHOD(GetHash)() CONST
		{
			if (hash == 0)
			{
				hash = HashCombine(num_render_targets, HashCombine((UInt64)depth_stencil_format, HashCombine((UInt64)sample_count, HashCombine( is_enable_vrs, is_read_only_dsv))));
				for (UInt8 rt = 0; rt < num_render_targets; ++rt)
					hash = HashCombine(hash, (UInt64)render_target_formats[rt]);
			}
			return hash;
		}

		UInt32 num_render_targets = 0;
		UInt32 sample_count = 1;
		Bool is_enable_vrs = false;
		Bool is_read_only_dsv = false;
		ENUM_TEXTURE_FORMAT depth_stencil_format = ENUM_TEXTURE_FORMAT::None;
		ENUM_TEXTURE_FORMAT render_target_formats[MYRENDER_MAX_RENDER_TARGETS] = { ENUM_TEXTURE_FORMAT::None };
	private:
		mutable UInt64 hash = 0;
		MYRENDERER_END_STRUCT

#pragma region METHOD
public:
	VK_RenderPassManager(VK_Device* in_device) ;
	VK_RenderPassManager(CONST VK_RenderPassManager&) DELETE;
	VK_RenderPassManager(VK_RenderPassManager&&) DELETE;
	VK_RenderPassManager& operator=(CONST VK_RenderPassManager&) DELETE;
	VK_RenderPassManager& operator=(VK_RenderPassManager&&) DELETE;

	~VK_RenderPassManager();

	VK_RenderPass* METHOD(GetRenderPass)(CONST RenderPassCacheKey& key);

	void METHOD(Destroy)();
protected:
private:

#pragma endregion

#pragma region MEMBER
public:
	
protected:
	struct RenderPassCacheKeyHash
	{
		UInt64 operator()(CONST RenderPassCacheKey& key) CONST
		{
			return key.GetHash();
		}
	};
	VK_Device* device;
	Map<RenderPassCacheKey, VK_RenderPass*, RenderPassCacheKeyHash> render_pass_cache;
private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
