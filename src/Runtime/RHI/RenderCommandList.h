#pragma once


#ifndef _RENDERCOMMANDLIST_
#define _RENDERCOMMANDLIST_
#include "Core/ConstDefine.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_STRUCT(DrawAttribute)
public:
	UInt32                                    vertexCount = 0;
	UInt32                                    instanceCount = 0;
	UInt32                                    firstVertex = 0;
	UInt32                                    firstInstance = 0;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_NAMESPACE(RHI)
class RenderPipelineState;
class ShaderResourceBinding;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CommandList,public RenderResource)

#pragma region METHOD
public:
	CommandList() MYDEFAULT;
	VIRTUAL ~CommandList() MYDEFAULT;
	VIRTUAL void METHOD(SetGraphicsPipeline)(RenderPipelineState* pipeline_state) PURE;
	VIRTUAL void METHOD(SetComputePipeline)(RenderPipelineState* pipeline_state) PURE;
	VIRTUAL void METHOD(SetRenderTarget)(CONST Vector<Texture*>& render_targets, Texture* depth_stencil, CONST Vector<ClearValue>& clear_values, Bool has_dsv_clear_value) PURE;
	VIRTUAL void METHOD(SetShaderResourceBinding)(ShaderResourceBinding* srb) PURE;
	VIRTUAL void METHOD(Draw)(CONST DrawAttribute& draw_attr) PURE;
	VIRTUAL void METHOD(Dispatch)(UInt32 groupX, UInt32 groupY, UInt32 groupZ) PURE;
	// --   Combined compute dispatch convenience
	VIRTUAL void METHOD(ComputeDispatch)(RenderPipelineState* pipeline, ShaderResourceBinding* srb, UInt32 groupX, UInt32 groupY, UInt32 groupZ) {}
	VIRTUAL void METHOD(SetPushConstants)(UInt32 offset, UInt32 size, const void* data) PURE;
	// --   Stage-aware push constants overload
	VIRTUAL void METHOD(SetPushConstants)(UInt32 offset, UInt32 size, const void* data, ENUM_SHADER_STAGE stage) {}
	VIRTUAL void METHOD(TransitionTextureState)(Texture* texture, CONST ENUM_RESOURCE_STATE& required_state) PURE;
	VIRTUAL void METHOD(ClearTexture)(Texture* texture,Vector<float> clear_value= Vector<float>(4,0.0f)) PURE;

	// --   RHI barrier API
	VIRTUAL void METHOD(ResourceBarrier)(ENUM_RESOURCE_STATE src_state, ENUM_RESOURCE_STATE dst_state) {}
	VIRTUAL void METHOD(MemoryBarrier)(ENUM_SHADER_STAGE src_stage, ENUM_SHADER_STAGE dst_stage, ENUM_RESOURCE_STATE src_access, ENUM_RESOURCE_STATE dst_access) {}
	// --  

	// --   Command list lifecycle
	VIRTUAL void METHOD(Begin)() PURE;
	VIRTUAL void METHOD(End)() PURE;
	// --  

	VIRTUAL void METHOD(BeginUI)() PURE;
	VIRTUAL void METHOD(EndUI)() PURE;

	// --   GPU timestamp query for per-pass timing
	VIRTUAL void METHOD(WriteTimestamp)(UInt32 query_index) {}
private:

protected:
#pragma endregion

#pragma region MEMBER
public:

private:

protected:
#pragma endregion

MYRENDERER_END_CLASS



MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
