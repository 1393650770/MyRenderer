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

// ---- RHI Command recording ----
// When enable_rhi_thread=true, commands are recorded into structs
// and replayed on the RHI thread. When bypass=true (default),
// commands execute immediately on the calling thread.

enum class RHICommandType : UInt8
{
	SetGraphicsPipeline,
	SetComputePipeline,
	SetRenderTarget,
	SetSRB,
	Draw,
	Dispatch,
	TransitionTexture,
	ClearTexture,
	ResourceBarrier,
	FlushBarriers,
	SetPushConstants,
	MapBuffer,
	UnmapBuffer,
	CopyBuffer,
	CopyBufferToImage,
	Begin,
	End,
	BeginRenderPass,
	EndRenderPass,
	BeginUI,
	EndUI,
	WriteTimestamp,
};

struct RHICommand
{
	RHICommandType type;
	RHICommand(RHICommandType t) : type(t) {}
	virtual ~RHICommand() MYDEFAULT;
};

// ---- Command structs for recording ----
struct RHICmdDraw : RHICommand {
	DrawAttribute attr;
	RHICmdDraw(CONST DrawAttribute& a) : RHICommand(RHICommandType::Draw), attr(a) {}
};
struct RHICmdDispatch : RHICommand {
	UInt32 gx, gy, gz;
	RHICmdDispatch(UInt32 x, UInt32 y, UInt32 z) : RHICommand(RHICommandType::Dispatch), gx(x), gy(y), gz(z) {}
};
struct RHICmdSetGraphicsPipeline : RHICommand {
	RenderPipelineState* pso;
	RHICmdSetGraphicsPipeline(RenderPipelineState* p) : RHICommand(RHICommandType::SetGraphicsPipeline), pso(p) {}
};
struct RHICmdSetComputePipeline : RHICommand {
	RenderPipelineState* pso;
	RHICmdSetComputePipeline(RenderPipelineState* p) : RHICommand(RHICommandType::SetComputePipeline), pso(p) {}
};
struct RHICmdSetSRB : RHICommand {
	ShaderResourceBinding* srb;
	RHICmdSetSRB(ShaderResourceBinding* s) : RHICommand(RHICommandType::SetSRB), srb(s) {}
};
struct RHICmdSetRenderTarget : RHICommand {
	Vector<Texture*> rtvs;
	Texture* dsv;
	Vector<ClearValue> clear_values;
	Bool has_dsv_clear;
	RHICmdSetRenderTarget(CONST Vector<Texture*>& r, Texture* d, CONST Vector<ClearValue>& c, Bool h) : RHICommand(RHICommandType::SetRenderTarget), rtvs(r), dsv(d), clear_values(c), has_dsv_clear(h) {}
};
struct RHICmdTransitionTexture : RHICommand {
	Texture* texture;
	ENUM_RESOURCE_STATE state;
	RHICmdTransitionTexture(Texture* t, CONST ENUM_RESOURCE_STATE& s) : RHICommand(RHICommandType::TransitionTexture), texture(t), state(s) {}
};
struct RHICmdClearTexture : RHICommand {
	Texture* texture;
	Vector<float> clear_value;
	RHICmdClearTexture(Texture* t, Vector<float>&& cv) : RHICommand(RHICommandType::ClearTexture), texture(t), clear_value(std::move(cv)) {}
};
struct RHICmdResourceBarrier : RHICommand {
	ENUM_RESOURCE_STATE src, dst;
	RHICmdResourceBarrier(ENUM_RESOURCE_STATE s, ENUM_RESOURCE_STATE d) : RHICommand(RHICommandType::ResourceBarrier), src(s), dst(d) {}
};
struct RHICmdFlushBarriers : RHICommand {
	// Cached image memory barrier data (platform-agnostic)
	struct CachedImageBarrier {
		UInt64 image;       // VkImage handle
		Int    old_layout;
		Int    new_layout;
		UInt32 src_access;
		UInt32 dst_access;
		UInt32 aspect_mask;
		UInt32 base_mip;
		UInt32 level_count;
		UInt32 base_layer;
		UInt32 layer_count;
	};
	Vector<CachedImageBarrier> image_barriers;
	UInt32 src_stages = 0;
	UInt32 dst_stages = 0;
	Bool   has_mem_barrier = false;
	UInt32 mem_src_access = 0;
	UInt32 mem_dst_access = 0;

	RHICmdFlushBarriers() : RHICommand(RHICommandType::FlushBarriers) {}
};
struct RHICmdSetPushConstants : RHICommand {
	UInt32 offset, size;
	Vector<UInt8> data;
	RHICmdSetPushConstants(UInt32 o, UInt32 s, const void* d) : RHICommand(RHICommandType::SetPushConstants), offset(o), size(s), data((UInt8*)d, (UInt8*)d + s) {}
};
struct RHICmdCopyBuffer : RHICommand {
	UInt64 src_id, dst_id;
	UInt32 region_count;
	Vector<UInt8> regions;
	RHICmdCopyBuffer(UInt64 s, UInt64 d, UInt32 c, const void* r, size_t sz) : RHICommand(RHICommandType::CopyBuffer), src_id(s), dst_id(d), region_count(c), regions((UInt8*)r, (UInt8*)r + sz) {}
};
struct RHICmdCopyBufferToImage : RHICommand {
	UInt64 src_buffer;   // VkBuffer handle
	UInt64 dst_image;    // VkImage handle
	Int    image_layout; // VkImageLayout
	UInt32 region_count;
	Vector<UInt8> regions; // VkBufferImageCopy[] raw data
	RHICmdCopyBufferToImage(UInt64 s, UInt64 d, Int l, UInt32 c, const void* r, size_t sz)
		: RHICommand(RHICommandType::CopyBufferToImage), src_buffer(s), dst_image(d)
		, image_layout(l), region_count(c), regions((UInt8*)r, (UInt8*)r + sz) {}
};

struct RHICmdBegin : RHICommand {
	RHICmdBegin() : RHICommand(RHICommandType::Begin) {}
};
struct RHICmdEnd : RHICommand {
	RHICmdEnd() : RHICommand(RHICommandType::End) {}
};
struct RHICmdBeginRenderPass : RHICommand {
	UInt64 render_pass;    // VkRenderPass handle
	UInt64 frame_buffer;   // VkFramebuffer handle
	UInt32 width;
	UInt32 height;
	UInt32 clear_count;
	// Clear values stored as raw data (VkClearValue is 16 bytes with union)
	Vector<UInt8> clear_value_data;
	RHICmdBeginRenderPass(UInt64 rp, UInt64 fb, UInt32 w, UInt32 h, UInt32 cc, const void* cv)
		: RHICommand(RHICommandType::BeginRenderPass), render_pass(rp), frame_buffer(fb)
		, width(w), height(h), clear_count(cc)
	{
		if (cc > 0 && cv)
			clear_value_data.assign((UInt8*)cv, (UInt8*)cv + cc * 16); // sizeof(VkClearValue) == 16
	}
};
struct RHICmdBeginUI : RHICommand { RHICmdBeginUI() : RHICommand(RHICommandType::BeginUI) {} };
struct RHICmdEndUI : RHICommand { RHICmdEndUI() : RHICommand(RHICommandType::EndUI) {} };
struct RHICmdWriteTimestamp : RHICommand {
	UInt32 index;
	RHICmdWriteTimestamp(UInt32 i) : RHICommand(RHICommandType::WriteTimestamp), index(i) {}
};
struct RHICmdUnmapBuffer : RHICommand {
	class Buffer* buffer;
	RHICmdUnmapBuffer(class Buffer* b) : RHICommand(RHICommandType::UnmapBuffer), buffer(b) {}
};

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
	VIRTUAL void METHOD(ComputeDispatch)(RenderPipelineState* pipeline, ShaderResourceBinding* srb, UInt32 groupX, UInt32 groupY, UInt32 groupZ) {}
	VIRTUAL void METHOD(SetPushConstants)(UInt32 offset, UInt32 size, const void* data) PURE;
	VIRTUAL void METHOD(SetPushConstants)(UInt32 offset, UInt32 size, const void* data, ENUM_SHADER_STAGE stage) {}
	VIRTUAL void METHOD(TransitionTextureState)(Texture* texture, CONST ENUM_RESOURCE_STATE& required_state) PURE;
	VIRTUAL void METHOD(ClearTexture)(Texture* texture,Vector<float> clear_value= Vector<float>(4,0.0f)) PURE;

	VIRTUAL void METHOD(ResourceBarrier)(ENUM_RESOURCE_STATE src_state, ENUM_RESOURCE_STATE dst_state) {}
	VIRTUAL void METHOD(MemoryBarrier)(ENUM_SHADER_STAGE src_stage, ENUM_SHADER_STAGE dst_stage, ENUM_RESOURCE_STATE src_access, ENUM_RESOURCE_STATE dst_access) {}

	VIRTUAL void METHOD(Begin)() PURE;
	VIRTUAL void METHOD(End)() PURE;

	VIRTUAL void METHOD(BeginUI)() PURE;
	VIRTUAL void METHOD(EndUI)() PURE;

	VIRTUAL void METHOD(WriteTimestamp)(UInt32 query_index) {}

	// ---- RHI Thread support ----
	void METHOD(SetBypass)(Bool b) { bypass = b; }
	Bool METHOD(IsBypass)() CONST { return bypass; }
	Vector<std::unique_ptr<RHICommand>>& METHOD(GetRecordedCommands)() { return recorded_commands; }
	VIRTUAL void METHOD(Replay)(){}

protected:
	Vector<std::unique_ptr<RHICommand>> recorded_commands;
	Bool bypass = true;

private:

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
