#pragma once
#ifndef _RENDER_ENUM_
#define _RENDER_ENUM_
#include "Core/ConstDefine.h"

namespace MXRender
{

	/// Renderer types:
	enum class ENUM_RENDER_API_TYPE
	{
		None = 0,
		Agc,
		Direct3D9,
		Direct3D11,
		Direct3D12,
		Gnm,
		Metal,
		Nvn,
		OpenGLES,
		OpenGL,
		Vulkan,
		WebGPU,

		Count
	};

	enum class ENUM_BUFFER_TYPE
	{
		None = 0,
		Static,
		Dynamic,
		Index,
		Vertex,
		Staging,
		Uniform,
		Storage,
		Indirect,
		Count
	};

	enum ENUM_SHADER_STAGE
	{
		Shader_Vertex = 0,
		Shader_Pixel = 1,

		Shader_Geometry = 2,

		Shader_Hull = 3,

		Shader_Domain = 4,

		Shader_Compute = 5,

		NumStages,

		MaxNumSets = 8,

		Invalid = -1,
	};

	enum class ENUM_RENDER_DATA_TYPE
	{
		None = 0,
		Float,
		Half,
		Mat3,
		Mat4,
		Int,
		Uint8,
		Uint10,
		Int16,
		Bool,

		Count
	};

	enum class ENUM_RENDER_DATA_USAGE_TYPE
	{
		NONE = 0,
		STATIC_DRAW,
		DYNAMIC_DRAW,

		COUNT
	};

	struct ENUM_RENDER_ATTRIBUTE_TYPE
	{
		enum Enum
		{
			None = 0,
			Position = 1,
			Normal = 2,
			Tangent = 3,
			Bitangent = 4,
			Color0 = 5,
			Color1 = 6,
			Color2 = 7,
			Color3 = 8,
			Indices = 9,
			Weight = 10,
			TexCoord0 = 11,
			TexCoord1 = 12,
			TexCoord2 = 13,
			TexCoord3 = 14,
			TexCoord4 = 15,
			TexCoord5 = 16,
			TexCoord6 = 17,
			TexCoord7 = 18,

			Count = 19,
		};
	};

	enum class ENUM_TEXTURE_TYPE
	{
		ENUM_TYPE_NOT_VALID = 0,
		ENUM_TYPE_2D,
		ENUM_TYPE_2D_MULTISAMPLE,
		ENUM_TYPE_2D_ARRAY,
		ENUM_TYPE_2D_DEPTH,
		ENUM_TYPE_CUBE_MAP,
		ENUM_TYPE_2D_DYNAMIC,
		ENUM_TYPE_3D
	};

	enum class ENUM_TEXTURE_USAGE_TYPE :UInt32
	{
		ENUM_TYPE_NOT_VALID = 0,
		ENUM_TYPE_COLOR_ATTACHMENT = 1 << 0,
		ENUM_TYPE_PRESENT_SWAPCHAIN = 2 << 1,
		ENUM_TYPE_COPY = 3 << 2,
		ENUM_TYPE_SHADERRESOURCE = 4 << 3,
		ENUM_TYPE_DEPTH_ATTACHMENT = 5 << 4,
		ENUM_TYPE_DEPTH_ATTACHMENT_READ_ONLY = 6 << 5,
		ENUM_TYPE_DEPTH_ATTACHMENT_WRITE_ONLY = 7 << 6,
	};
	ENUM_CLASS_FLAGS(ENUM_TEXTURE_USAGE_TYPE)

	enum class ENUM_QUEUE_TYPE :UInt8
	{
		NOT_VALID = 0,
		GRAPHICS,
		COMPUTE,
		TRANSFER,
		PRESENT,
		COUNT
	};

	/// Texture formats:
	enum class ENUM_TEXTURE_FORMAT: UInt32
	{
		None = 0,
		BC1,
		BC1A,
		BC2,
		BC3,
		BC4,
		BC5,
		BC6H,
		BC7,
		ETC1,
		ETC2,
		ETC2A,
		ETC2A1,
		PTC12,
		PTC14,
		PTC12A,
		PTC14A,
		PTC22,
		PTC24,
		ATC,
		ATCE,
		ATCI,
		ASTC4x4,
		ASTC5x5,
		ASTC6x6,
		ASTC8x5,
		ASTC8x6,
		ASTC10x5,

		Unknown,

		R1,
		A8,
		R8,
		R8I,
		R8U,
		R8S,
		R16,
		R16I,
		R16U,
		R16F,
		R16S,
		R32I,
		R32U,
		R32F,
		RG8,
		RG8I,
		RG8U,
		RG8S,
		RG16,
		RG16I,
		RG16U,
		RG16F,
		RG16S,
		RG32I,
		RG32U,
		RG32F,
		RGB8,
		RGB8I,
		RGB8U,
		RGB8S,
		RGB9E5F,
		RGB16I,
		RGB16U,
		RGB16F,
		RGB32I,
		RGB32U,
		RGB32F,
		BGRA8,
		RGBA8,
		RGBA8I,
		RGBA8U,
		RGBA8S,
		RGBA16,
		RGBA16I,
		RGBA16U,
		RGBA16F,
		RGBA16S,
		RGBA32I,
		RGBA32U,
		RGBA32F,
		R5G6B5,
		RGBA4,
		RGB5A1,
		RGB10A2,
		RG11B10F,

		UnknownDepth, // Depth formats below.

		D16,
		D24,
		D24S8,
		D32,
		D32FS8,
		D16F,
		D24F,
		D32F,
		D0S8,

		Count
	};

	enum class ENUM_FRAMEBUFFER_TYPE
	{
		ENUM_TYPE_INVALID = 0,
		ENUM_TYPE_BASIC,
		ENUM_TYPE_RGBF1_DEPTH,
		ENUM_TYPE_RGBF2_DEPTH,
		ENUM_TYPE_RGBF3_DEPTH,
		ENUM_TYPE_MSAA,
		ENUM_TYPE_COLOR,
		ENUM_TYPE_RED,
		ENUM_TYPE_COLOR_FLOAT,
		ENUM_TYPE_DEPTH,
		ENUM_TYPE_CUBE_DEPTH,
		ENUM_TYPE_GBUFFER,
		ENUM_TYPE_RAYTRACING,
		ENUM_TYPE_RTX,
		ENUM_TYPE_DYNAMIC_COLOR,
	};

	enum class ENUM_PASS_TYPE : UInt8
	{
		ENUM_TYPE_INVALID = 0,
		ENUM_PASS_COLOR,
		ENUM_PASS_DEPTH,
	};

	enum class ENUM_STENCIL_FUNCTION : UInt8
	{
		ENUM_NONE = 0,
		ENUM_ALWAYS,
		ENUM_NOTEQUAL,
		ENUM_NEVER,
		ENUM_LESS,
		ENUM_LEQUAL,
		ENUM_GREATER,
		ENUM_EQUAL,
		ENUM_NOT_EQUAL,
		ENUM_LESSOREQUAL,
		ENUM_GREATEROREQUAL,
		ENUM_COUNT
	};

	enum class ENUM_STENCIL_OPERATIOON : UInt8
	{
		ENUM_NONE = 0,
		ENUM_KEEP,
		ENUM_ZERO,
		ENUM_REPLACE,
		ENUM_INCREMENT_AND_CLAMP,
		ENUM_DECREMENT_AND_CLAMP,
		ENUM_INVERT,
		ENUM_INCREMENT_AND_WRAP,
		ENUM_DECREMENT_AND_WRAP,
		ENUM_COUNT
	};

	enum class ENUM_DEPTH_FUNCTION : UInt8
	{
		ENUM_NONE = 0,
		ENUM_EQUAL,
		ENUM_LEAQUAL,
		ENUM_LESS,
		ENUM_GREATER,
		ENUM_GEQUAL,
		ENUM_NOTEQUAL,
		ENUM_ALWAYS,

		ENUM_COUNT
	};

	enum class ENUM_BLEND_EQUATION : UInt8
	{
		ENUM_NONE = 0,
		ENUM_ADD,
		ENUM_SUB,
		ENUM_REVERSE_SUB,
		ENUM_MIN,
		ENUM_MAX,
		ENUM_COUNT
	};
	enum class ENUM_COLOR_MASK : UInt8
	{
		Red = 1,
		Green = 2,
		Blue = 4,
		Alpha = 8,
		All = 0xF
	};

	enum class ENUM_BLEND_FACTOR : UInt8
	{
		ENUM_NONE = 0,
		ENUM_ZERO,
		ENUM_ONE,
		ENUM_SRC_COLOR,
		ENUM_DST_COLOR,
		ENUM_SRC_ALPHA,
		ENUM_DST_ALPHA,
		ENUM_ONE_MINUS_SRC_COLOR,
		ENUM_ONE_MINUS_DST_COLOR,
		ENUM_ONE_MINUS_SRC_ALPHA,
		ENUM_ONE_MINUS_DST_ALPHA,
		EUNUM_SRC_ALPHA_SATURATE,
		ENUM_CONSTANT_COLOR,
		ENUM_INV_CONSTANT_COLOR,
		ENUM_CONSTANT_ALPHA,
		ENUM_INV_CONSTANT_ALPHA,
		ENUM_COUNT
	};

	enum class ENUM_RASTER_FILLMODE : UInt8
	{
		Solid,
		Wireframe,

		// Vulkan names
		Fill = Solid,
		Line = Wireframe
	};

	enum class ENUM_RASTER_CULLMODE : UInt8
	{
		Back,
		Front,
		None
	};

	enum class ENUM_PRIMITIVE_TYPE : UInt8
	{
		PointList,
		LineList,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency,
		PatchList
	};

	enum class ENUM_VERTEX_ATTRIBUTE_FORMAT : UInt8
	{
		None,
		Float,
		Float2,
		Float3,
		Float4,
		Byte4,
		Byte4N,
		UByte4,
		UByte4N,
		Short2,
		Short2N,
		Short4,
		Short4N,
		UShort2,
		UShort2N,
		UShort4,
		UShort4N,
		UInt10N2,
		Count
	};

	enum class ENUM_TEXTURE_COMPONENT_FORMAT : UInt8
	{
		None,
		Float,
		SNorm,
		UNorm,
		UNormSRGB,
		SInt,
		UInt,
		Depth,
		DepthStencil,
		Compound,
		Compressed,
		Count
	};

	enum class ENUM_TEXTURE_FILTER : UInt8
	{
		None = 0,
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear,
		Count
	};

	enum class ENUM_BINDING_RESOURCE_TYPE : UInt8
	{
		Invalid = 0,
		UniformBuffer,
		StorageBuffer,
		SampledTexture,
		StorageTexture,
		Sampler,
		UniformBufferDynamic,
		StorageBufferDynamic,
		Count
	};

	enum class ENUM_RENDERPASS_ATTACHMENT_LOAD_OP : UInt8
	{
		Load,
		Clear,
		DISCARD,
		Count
	};

	enum class ENUM_RENDERPASS_ATTACHMENT_STORE_OP : UInt8
	{
		Store,
		DISCARD,
		Count
	};

	enum class ENUM_RESOURCE_STATE : UInt64
	{
		Invalid = 0,
		Undefined = 1 << 0,
		VertexBuffer  = 1<<1,
		ConstantBuffer=1<<2,
		IndexBuffer=1<<3,
		RenderTarget=1<<4,
		UnorderedAccess=1<<5,
		DepthWrite=1<<6,
		DepthRead=1<<7,
		ShaderResource=1<<8,
		StreamOut=1<<9,
		IndirectArgument=1<<10,
		CopyDest=1<<11,
		CopySource=1<<12,
		ResolveDest=1<<13,
		ResolveSource=1<<14,
		InputAttachment=1<<15,
		Present=1<<16,
		BuildAsRead=1<<17,
		BuildAsWrite=1<<18,
		Raytracing=1<<19,
		Common=1<<20,
		ShaderRate = 1 << 21,
		Count = ShaderRate,
		GenerricRead = VertexBuffer | IndexBuffer | ConstantBuffer | ShaderResource | IndirectArgument | CopySource,
	};
	ENUM_CLASS_FLAGS(ENUM_RESOURCE_STATE)


	MYRENDERER_BEGIN_STRUCT(StencilOpDesc)
		ENUM_STENCIL_OPERATIOON fail_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_OPERATIOON depth_fail_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_OPERATIOON pass_op{ ENUM_STENCIL_OPERATIOON::ENUM_KEEP };
		ENUM_STENCIL_FUNCTION func{ ENUM_STENCIL_FUNCTION::ENUM_ALWAYS };
	MYRENDERER_END_STRUCT


	enum class ENUM_VERTEX_INPUTRATE : UInt8
	{
		None=0,
		PerVertex,
		PerInstance,
		Count
	};


} // namespace name
#endif // !_MXRENDER_ENUM_