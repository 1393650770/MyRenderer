#include "RenderPass.h"
#include "Core/TypeHash.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)


RenderPass::RenderPass(CONST RenderPassDesc& in_desc): desc(in_desc)
{

}

RenderPassCacheKey::RenderPassCacheKey(UInt8 in_num_render_targets, const ENUM_TEXTURE_FORMAT* in_render_target_formats, ENUM_TEXTURE_FORMAT in_depth_stencil_format, UInt8 in_sample_count, Bool in_is_enable_vrs, Bool in_is_read_only_dsv) :
	num_render_targets(in_num_render_targets),
	sample_count(in_sample_count),
	is_enable_vrs(in_is_enable_vrs),
	is_read_only_dsv(in_is_read_only_dsv),
	depth_stencil_format(in_depth_stencil_format)
{
	for (UInt8 rt = 0; rt < num_render_targets; ++rt)
		render_target_formats[rt] = in_render_target_formats[rt];
}
UInt64 RenderPassCacheKey::GetHash() CONST
{
	if (hash == 0)
	{
		//hash = HashCombine(num_render_targets, HashCombine((UInt64)depth_stencil_format, HashCombine((UInt64)sample_count, HashCombine(is_enable_vrs, is_read_only_dsv))));
		//for (UInt8 rt = 0; rt < num_render_targets; ++rt)
		//	hash = HashCombine(hash, (UInt64)render_target_formats[rt]);
		hash = CityHash64((Char*)this, sizeof(RenderPassCacheKey));
	}
	return hash;
}
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE