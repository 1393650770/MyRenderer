#pragma once


#ifndef _RENDERROURCE_
#define _RENDERROURCE_
#include "RHI/RenderEnum.h"
#include <cstdint>
#include <memory>
#include <string>
#include "Core/BaseObject.h"
#include <algorithm>
#include <utility>



#define MYRENDER_MAX_RENDER_TARGETS 8

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
class Shader;
class RenderPass;

MYRENDERER_BEGIN_STRUCT(BufferDesc)
public:
	UInt32 size = 0;
	UInt32 stride = 0;
	ENUM_BUFFER_TYPE type = ENUM_BUFFER_TYPE::None;
	BufferDesc() = default;
	BufferDesc(const BufferDesc& other)
	{
		*this = other;
	}

	BufferDesc& operator=(const BufferDesc& other)
	{
		size = other.size;
		stride = other.stride;
		type = other.type;
		return *this;
	}

MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ShaderDesc)
public:
	ShaderDesc() DEFAULT;
	ShaderDesc(ENUM_SHADER_STAGE in_shader_type) : shader_type(in_shader_type) {}
	ENUM_SHADER_STAGE shader_type = ENUM_SHADER_STAGE::Invalid;
	String debug_name;
	String entry_name = "main";
	String shader_name;
MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_STRUCT(ShaderDataPayload)
public:
	ShaderDataPayload() DEFAULT;
	ShaderDataPayload(CONST ShaderDataPayload& other);

	ShaderDataPayload& operator=(CONST ShaderDataPayload& other)
	{
		return *this;
	}

	Vector<UInt32> data;
	~ShaderDataPayload()
	{
		data.clear();
	}
	MYRENDERER_BEGIN_STRUCT(ShaderReflectionOverrides)
	CONST Char* name = nullptr;
	ENUM_BINDING_RESOURCE_TYPE overriden_type = ENUM_BINDING_RESOURCE_TYPE::Invalid;
	MYRENDERER_END_STRUCT

MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_STRUCT(FrameBufferDesc)
	UInt32 width = 0;
	UInt32 height = 0;
	UInt8 layer = 1;
	RenderPass* render_pass= nullptr;
	//last element is the depth stencil attachment
	Vector<RHI::Texture*> render_targets;
MYRENDERER_END_STRUCT



MYRENDERER_BEGIN_STRUCT(RenderPassDesc)
public:

	MYRENDERER_BEGIN_STRUCT(RenderPassAttatchmentDesc)
	public:
		ENUM_TEXTURE_FORMAT 			format           ;
		UInt8                   sample_count     =1;
		ENUM_RENDERPASS_ATTACHMENT_LOAD_OP      load_op          ;
		ENUM_RENDERPASS_ATTACHMENT_STORE_OP     store_op;
		ENUM_RENDERPASS_ATTACHMENT_LOAD_OP      stencil_load_op  ;
		ENUM_RENDERPASS_ATTACHMENT_STORE_OP     stencil_store_op;
		ENUM_RESOURCE_STATE          initial_state ;
		ENUM_RESOURCE_STATE          final_state;
	MYRENDERER_END_STRUCT

	MYRENDERER_BEGIN_STRUCT(RenderPassAttachmentReferenceDesc)
	public:
		UInt8 attachment_index = 0;
		ENUM_RESOURCE_STATE state = ENUM_RESOURCE_STATE::Invalid;
	MYRENDERER_END_STRUCT

	Vector<RenderPassAttatchmentDesc> attachments;
	Vector<RenderPassAttachmentReferenceDesc> attachment_refs;
MYRENDERER_END_STRUCT


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


protected:

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
	Bool is_valid() CONST { return spirv.size() > 0; }
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_STRUCT(BlenderState)

	MYRENDERER_BEGIN_STRUCT(RenderTarget)
	public:
		Bool        blend_enable = false;
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
			return  blend_enable == rhs.blend_enable &&
				src_color == rhs.src_color &&
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
	
Bool depth_test_enable{ true };
Bool depth_write_enable{ true };
ENUM_STENCIL_FUNCTION depth_func{ ENUM_STENCIL_FUNCTION::ENUM_ALWAYS };
Bool stencil_test_enable{ false };
UInt8 stencil_read_mask{ 0xFF };
UInt8 stencil_write_mask{ 0xFF };
UInt8 stencil_ref{ 0 };
StencilOpDesc front_face_stencil{};
StencilOpDesc back_face_stencil{};
Bool depth_bounds_test_enable{ false };
Float32 min_depth_bounds { 0.0f };
Float32 max_depth_bounds { 0.0f };
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(RasterizerState)
	RasterizerState()DEFAULT;
	~RasterizerState()DEFAULT;

	ENUM_RASTER_CULLMODE cull_mode{ ENUM_RASTER_CULLMODE::Back };
	ENUM_RASTER_FILLMODE fill_mode{ ENUM_RASTER_FILLMODE::Solid };
	Bool front_counter_clockwise{ false };
	Bool depth_clip_enable{ false };
	Bool scissor_enable{ false };
	Bool antialiased_line_enable{ false };
	Float32 depth_bias{ 0.0f };
	Float32 depth_bias_clamp{ 0.0f };
	Float32 depth_bias_slope_scaled{ 0.0f };
	Float32 line_width{ 1.0f };
	UInt8 sample_count { 1 };

	constexpr Bool operator == (CONST RasterizerState& rhs) CONST
	{
		return cull_mode == rhs.cull_mode &&
			fill_mode == rhs.fill_mode &&
			front_counter_clockwise == rhs.front_counter_clockwise &&
			depth_clip_enable == rhs.depth_clip_enable &&
			scissor_enable == rhs.scissor_enable &&
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
public:
	UInt32 binding{ 0 };
	UInt32 location{ 0 };
	ENUM_TEXTURE_FORMAT attribute_format{ ENUM_TEXTURE_FORMAT::None };
	ENUM_VERTEX_INPUTRATE input_rate{ ENUM_VERTEX_INPUTRATE::PerVertex };
	UInt32 offset{ 0 };

	VertexInputLayout() DEFAULT;
	~VertexInputLayout() DEFAULT;

	VertexInputLayout& operator= (CONST VertexInputLayout& rhs)
	{
		if (this != &rhs) // ×Ô¸³Öµ¼ì²é
		{
			binding = rhs.binding;
			location = rhs.location;
			attribute_format = rhs.attribute_format;
			input_rate = rhs.input_rate;
			offset = rhs.offset;
		}
		return *this;
	}
	VertexInputLayout(CONST VertexInputLayout& other)
	{
		*this = other;
	}

	VertexInputLayout(VertexInputLayout&& other) noexcept
		: binding(std::exchange(other.binding, 0)),
		location(std::exchange(other.location, 0)),
		attribute_format(std::exchange(other.attribute_format, ENUM_TEXTURE_FORMAT::None)),
		input_rate(std::exchange(other.input_rate, ENUM_VERTEX_INPUTRATE::PerVertex)),
		offset(std::exchange(other.offset, 0))
	{
	}

	Bool operator== (CONST VertexInputLayout& rhs) CONST
	{
		return binding == rhs.binding &&
			location == rhs.location &&
			attribute_format == rhs.attribute_format &&
			offset == rhs.offset &&
			input_rate == rhs.input_rate;
	};


MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphiPipelineState,public RenderResource)
public:
	RenderGraphiPipelineState() DEFAULT;
	VIRTUAL~RenderGraphiPipelineState() DEFAULT;

	Shader* shaders[ENUM_SHADER_STAGE::NumStages]{nullptr};
	ENUM_PRIMITIVE_TYPE primitive_topology{ ENUM_PRIMITIVE_TYPE::TriangleList };
	BlenderState* blend_state;
	DepthStencilState* depth_stencil_state;
	RasterizerState* raster_state;
	Vector<VertexInputLayout> vertex_input_layout;
	RenderPass* render_pass{ nullptr };

	constexpr Bool operator == (CONST RenderGraphiPipelineState& rhs) CONST
	{
		for (UInt32 i = 0; i < ENUM_SHADER_STAGE::NumStages; ++i)
		{
			if (shaders[i] != rhs.shaders[i])
				return false;
		}
		if(vertex_input_layout.size() != rhs.vertex_input_layout.size())
			return false;
		for (UInt32 i = 0; i < vertex_input_layout.size(); ++i)
		{
			if (vertex_input_layout[i] != rhs.vertex_input_layout[i])
				return false;
		}
		return 
			primitive_topology == rhs.primitive_topology &&
			blend_state == rhs.blend_state &&
			depth_stencil_state == rhs.depth_stencil_state &&
			raster_state == rhs.raster_state;
	};

	constexpr Bool operator != (CONST RenderGraphiPipelineState& rhs) CONST
	{
		return !(*this == rhs);
	};

	CONST Vector<VertexInputLayout>& GetSortedVertexInputLayout() CONST
	{
		static Vector<VertexInputLayout> sortedLayouts = vertex_input_layout;
		std::sort(sortedLayouts.begin(), sortedLayouts.end(), [](CONST VertexInputLayout& lhs, CONST VertexInputLayout& rhs)->Bool {
			if (lhs.input_rate != rhs.input_rate)
				return lhs.input_rate < rhs.input_rate;
			else if (lhs.binding != rhs.binding)
				return lhs.binding < rhs.binding;
			return lhs.location < rhs.location;
		});
		return sortedLayouts;
	}

	UInt64 GetHash() CONST;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE


#endif _RENDERROURCE_
