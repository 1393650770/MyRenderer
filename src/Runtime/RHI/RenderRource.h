#pragma once


#ifndef _RENDERROURCE_
#define _RENDERROURCE_
#include "RHI/RenderEnum.h"
#include <cstdint>
#include <memory>
#include <string>
#include "Core/BaseObject.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Shader;

static constexpr UInt32 g_max_render_targets = 8;
static constexpr UInt32 g_max_viewports = 16;
static constexpr UInt32 g_max_vertex_attributes = 16;
static constexpr UInt32 g_max_binding_layouts = 5;
static constexpr UInt32 g_max_bindings_per_layout = 128;
static constexpr UInt32 g_max_volatile_constant_buffers_per_layout = 6;
static constexpr UInt32 g_max_volatile_constant_buffers = 32;
static constexpr UInt32 g_max_push_constant_size = 128; // D3D12: root signature is 256 bytes max., Vulkan: 128 bytes of push constants guaranteed
static constexpr UInt32 c_ConstantBufferOffsetSizeAlignment = 256;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderResource,public IObject)
public:
	RenderResource() DEFAULT;
	VIRTUAL ~RenderResource() DEFAULT;
	VIRTUAL void METHOD(Release)();
protected:
	Bool is_valid{ false };
	UInt32 ref_count{ 1 };

private:
private:

protected:

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(TextureData)

public:
	UInt32 width{ 0 };
	UInt32 height{ 0 };
	UInt32 depth{ 0 };
	UInt32 mip_levels{ 0 };
	UInt32 array_layers{ 0 };
	void* pixels{ nullptr };

	ENUM_TEXTURE_FORMAT format{ ENUM_TEXTURE_FORMAT::Unknown };
	ENUM_TEXTURE_TYPE   type{ ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID };

	TextureData() DEFAULT;
	~TextureData()
	{
		if (pixels)
		{
			free(pixels);
			pixels = nullptr;
		}
	}
	Bool is_valid() CONST { return pixels != nullptr; }
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(ShaderData)

public:
	Vector<UInt32> spirv;
	ENUM_SHADER_STAGE stage{ ENUM_SHADER_STAGE::Invalid };

	ShaderData() DEFAULT;
	~ShaderData()
	{
		spirv.clear();
	}
	Bool is_valid() CONST { return spirv.size()>0; }
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_STRUCT(BlenderState)

	MYRENDERER_BEGIN_STRUCT(RenderTarget)
	public:
		ENUM_BLEND_FACTOR src_color{ ENUM_BLEND_FACTOR::ENUM_ONE };
		ENUM_BLEND_FACTOR dst_color{ ENUM_BLEND_FACTOR::ENUM_ZERO };
		ENUM_BLEND_FACTOR src_alpha{ ENUM_BLEND_FACTOR::ENUM_ONE };
		ENUM_BLEND_FACTOR dst_alpha{ ENUM_BLEND_FACTOR::ENUM_ZERO };
		ENUM_BLEND_EQUATION op_color{ ENUM_BLEND_EQUATION::ENUM_ADD };
		ENUM_BLEND_EQUATION op_alpha{ ENUM_BLEND_EQUATION::ENUM_ADD };
		ENUM_COLOR_MASK write_mask{ ENUM_COLOR_MASK::All };

		RenderTarget() DEFAULT;
		~RenderTarget() DEFAULT;

		constexpr Bool operator == (CONST RenderTarget& rhs) CONST
		{
			return src_color == rhs.src_color &&
				dst_color == rhs.dst_color &&
				src_alpha == rhs.src_alpha &&
				dst_alpha == rhs.dst_alpha &&
				op_color == rhs.op_color &&
				op_alpha == rhs.op_alpha &&
				write_mask == rhs.write_mask;
		};

		constexpr Bool operator != (CONST RenderTarget& rhs) CONST
		{
			return !(*this == rhs);
		};


	MYRENDERER_END_STRUCT

	Array<RenderTarget, g_max_render_targets> render_targets;
	Bool enable_alpha_to_coverage = false;

	constexpr Bool operator == (CONST BlenderState& rhs) CONST
	{
		if (enable_alpha_to_coverage != rhs.enable_alpha_to_coverage)
			return false;

		for (UInt32 i = 0; i < g_max_render_targets; ++i)
		{
			if (render_targets[i] != rhs.render_targets[i])
			{
				return false;
			}
		}
		return true;
	};

	constexpr Bool operator != (CONST BlenderState& rhs) CONST
	{
		return !(*this == rhs);
	};
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(DepthStencilState)
DepthStencilState()DEFAULT;
~DepthStencilState()DEFAULT;

	MYRENDERER_BEGIN_STRUCT(StencilOpDesc)
		ENUM_STENCIL_OPERATIOON fail_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_OPERATIOON depth_fail_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_OPERATIOON pass_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_FUNCTION func{ ENUM_STENCIL_FUNCTION::ENUM_ALWAYS };
	MYRENDERER_END_STRUCT
	
Bool depth_test_enable{ true };
Bool depth_write_enable{ true };
ENUM_STENCIL_FUNCTION depth_func{ ENUM_STENCIL_FUNCTION::ENUM_ALWAYS };
Bool stencil_test_enable{ false };
UInt8 stencil_read_mask{ 0xFF };
UInt8 stencil_write_mask{ 0xFF };
UInt8 stencil_ref{ 0 };
StencilOpDesc front_face_stencil{};
StencilOpDesc back_face_stencil{};

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(RasterizerState)
	RasterizerState()DEFAULT;
	~RasterizerState()DEFAULT;

	ENUM_RASTER_CULLMODE cull_mode{ ENUM_RASTER_CULLMODE::Back };
	ENUM_RASTER_FILLMODE fill_mode{ ENUM_RASTER_FILLMODE::Solid };
	Bool front_counter_clockwise{ false };
	Bool depth_clip_enable{ false };
	Bool scissor_enable{ false };
	Bool multisample_enable{ false };
	Bool antialiased_line_enable{ false };
	Float32 depth_bias{ 0.0f };
	Float32 depth_bias_clamp{ 0.0f };
	Float32 depth_bias_slope_scaled{ 0.0f };
	Float32 line_width{ 1.0f };

	constexpr Bool operator == (CONST RasterizerState& rhs) CONST
	{
		return cull_mode == rhs.cull_mode &&
			fill_mode == rhs.fill_mode &&
			front_counter_clockwise == rhs.front_counter_clockwise &&
			depth_clip_enable == rhs.depth_clip_enable &&
			scissor_enable == rhs.scissor_enable &&
			multisample_enable == rhs.multisample_enable &&
			antialiased_line_enable == rhs.antialiased_line_enable &&
			depth_bias == rhs.depth_bias &&
			depth_bias_clamp == rhs.depth_bias_clamp &&
			depth_bias_slope_scaled == rhs.depth_bias_slope_scaled &&
			line_width == rhs.line_width;
	};

	constexpr Bool operator != (CONST RasterizerState& rhs) CONST
	{
		return !(*this == rhs);
	};

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(VertexInputLayout)
	UInt32 binding{ 0 };
	UInt32 location{ 0 };
	ENUM_VERTEX_ATTRIBUTE_FORMAT format{ ENUM_VERTEX_ATTRIBUTE_FORMAT::None };
	UInt32 offset{ 0 };

	constexpr Bool operator == (CONST VertexInputLayout& rhs) CONST
	{
		return binding == rhs.binding &&
			location == rhs.location &&
			format == rhs.format &&
			offset == rhs.offset;
	};

	constexpr Bool operator != (CONST VertexInputLayout& rhs) CONST
	{
		return !(*this == rhs);
	};

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphiPipelineState,public RenderResource)
public:
	RenderGraphiPipelineState() DEFAULT;
	VIRTUAL~RenderGraphiPipelineState() DEFAULT;

	Shader* shaders;
	ENUM_PRIMITIVE_TYPE primitive_topology{ ENUM_PRIMITIVE_TYPE::TriangleList };
	BlenderState* blend_state;
	DepthStencilState* depth_stencil_state;
	RasterizerState* raster_state;
	VertexInputLayout* vertex_input_layout;

	constexpr Bool operator == (CONST RenderGraphiPipelineState& rhs) CONST
	{
		return shaders == rhs.shaders &&
			primitive_topology == rhs.primitive_topology &&
			blend_state == rhs.blend_state &&
			depth_stencil_state == rhs.depth_stencil_state &&
			raster_state == rhs.raster_state &&
			vertex_input_layout == rhs.vertex_input_layout;
	};

	constexpr Bool operator != (CONST RenderGraphiPipelineState& rhs) CONST
	{
		return !(*this == rhs);
	};
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE


#endif _RENDERROURCE_
