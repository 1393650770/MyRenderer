#include "RenderRource.h"
#include "Core/TypeHash.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

 void RenderResource::Release()
{
	if (--ref_count == 0)
		delete this;
}

UInt64 RenderGraphiPipelineStateDesc::GetHash() CONST
{
	UInt64 h = 0;
	for (UInt32 i = 0; i < ENUM_SHADER_STAGE::NumStages; ++i)
		h = HashCombine(h, (UInt64)shaders[i]);
	h = HashCombine(h, (UInt64)primitive_topology);
	h = HashCombine(h, (UInt64)raster_state.fill_mode);
	h = HashCombine(h, (UInt64)raster_state.cull_mode);
	h = HashCombine(h, (UInt64)raster_state.sample_count);
	h = HashCombine(h, (UInt64)depth_stencil_state.depth_test_enable);
	h = HashCombine(h, (UInt64)depth_stencil_state.depth_write_enable);
	h = HashCombine(h, (UInt64)depth_stencil_state.depth_func);
	h = HashCombine(h, (UInt64)blend_state.enable_alpha_to_coverage);
	for (UInt32 i = 0; i < (UInt32)render_targets.size(); ++i)
		h = HashCombine(h, (UInt64)render_targets[i]);
	h = HashCombine(h, (UInt64)depth_stencil_view);
	return h;
}

void RenderResource::AddRef()
{
	++ref_count;
}

void RenderResource::Realize() CONST
{
	CHECK_WITH_LOG(true, "Not implemented");
}

void RenderResource::DeRealize() CONST
{
	CHECK_WITH_LOG(true , "Not implemented");
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE