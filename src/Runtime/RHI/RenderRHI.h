#pragma once

#ifndef _RENDERRHI_
#define _RENDERRHI_
#include"RenderState.h"
#include"RenderEnum.h"
#include <string>
#include<vector>
#include<memory>
#include "../Core/ConstDefine.h"
#include "RenderRource.h"




MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS(RenderFactory)
public:
	Int render_api_version=0;
	Bool enable_render_debug=false;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderRHI,public RenderResource)

public:
	RenderRHI() DEFAULT;
	VIRTUAL~ RenderRHI() DEFAULT;

#pragma region INIT_MATHOD
	VIRTUAL void METHOD(Init)(RenderFactory* render_factory) PURE;

	VIRTUAL void METHOD(PostInit)() PURE;

	VIRTUAL void METHOD(Shutdown)() PURE;
#pragma endregion


#pragma region CREATE_RESOURCE
	VIRTUAL void METHOD(CreateVertexBuffer)(UInt size, UInt usage, UInt stride, void* data, RenderVertexBuffer** vertex_buffer) PURE;

	VIRTUAL void METHOD(CreateIndexBuffer)(UInt size, UInt usage, UInt stride, void* data, RenderIndexBuffer** index_buffer) PURE;

	VIRTUAL void METHOD(CreateVertexDeclaration)(const VertexElement* vertex_elements, UInt element_count, RenderVertexDeclaration** vertex_declaration) PURE;

	VIRTUAL void METHOD(CreateTexture2D)(UInt width, UInt height, UInt mip_levels, UInt usage, PixelFormat format, UInt sample_count, UInt sample_quality, void* data, RenderTexture2D** texture2d) PURE;

	VIRTUAL void METHOD(CreateTextureCube)(UInt size, UInt mip_levels, UInt usage, PixelFormat format, UInt sample_count, UInt sample_quality, void* data, RenderTextureCube** texture_cube) PURE;

	VIRTUAL void METHOD(CreateRenderTarget)(UInt width, UInt height, PixelFormat format, UInt multi_sample_count, UInt multi_sample_quality, Bool lockable, RenderTexture2D** render_target) PURE;

	VIRTUAL void METHOD(CreateDepthStencilSurface)(UInt width, UInt height, PixelFormat format, UInt multi_sample_count, UInt multi_sample_quality, Bool discard, RenderTexture2D** depth_stencil_surface) PURE;

	VIRTUAL void METHOD(CreateDepthStencilSurface)(UInt width, UInt height, PixelFormat format, UInt multi_sample_count, UInt multi_sample_quality, Bool discard, RenderTextureCube** depth_stencil_surface) PURE;

	VIRTUAL void METHOD(CreateDepthStencilSurface)(UInt width, UInt height, PixelFormat format, UInt multi_sample_count, UInt multi_sample_quality, Bool discard, RenderDepthStencilSurface** depth_stencil_surface) PURE;

	VIRTUAL void METHOD(CreateSamplerState)(const SamplerStateDesc* desc, RenderSamplerState** sampler_state) PURE;

	VIRTUAL void METHOD(CreateRasterizerState)(const RasterizerStateDesc* desc, RenderRasterizerState** rasterizer_state) PURE;

	VIRTUAL void METHOD(CreateDepthStencilState)(const DepthStencilStateDesc* desc, RenderDepthStencilState** depth_stencil_state) PURE;

	VIRTUAL void METHOD(CreateBlendState)(const BlendStateDesc* desc, RenderBlendState** blend_state) PURE;

	VIRTUAL void METHOD(CreateVertexShader)(const void* shader_byte_code, UInt byte_code_length, RenderVertexShader** vertex_shader) PURE;

	VIRTUAL void METHOD(CreatePixelShader)(const void* shader_byte
	

#pragma endregion

#pragma region DRAW
	VIRTUAL void METHOD(DrawPrimitive)(PrimitiveType primitive_type, UInt start_index, UInt primitive_count) PURE;

	VIRTUAL void METHOD(DrawIndexedPrimitive)(PrimitiveType primitive_type, UInt start_index, UInt primitive_count, UInt start_vertex, UInt vertex_count) PURE;

	VIRTUAL void METHOD(DrawPrimitiveUP)(PrimitiveType primitive_type, UInt primitive_count, const void* vertex_data, UInt vertex_stride) PURE;

	VIRTUAL void METHOD(DrawIndexedPrimitiveUP)(PrimitiveType primitive_type, UInt primitive_count, const void* index_data, UInt index_stride, const void* vertex_data, UInt vertex_stride) PURE;

	VIRTUAL void METHOD(GetRenderApi)() PURE;
#pragma endregion

private:

protected:


MYRENDERER_END_CLASS

extern RenderRHI* g_render_rhi;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

