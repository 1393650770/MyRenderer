#pragma once
#ifndef _RENDER_ENUM_
#define _RENDER_ENUM_


namespace MXRender
{
	/// Renderer types:
	enum class ENUM_RENDER_API_TYPE
	{
		None=0,        
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
		ENUM_TYPE_2D_DEPTH,
		ENUM_TYPE_CUBE_MAP,
		ENUM_TYPE_2D_DYNAMIC,
	};

	enum class ENUM_SHADER_TYPE
	{
		ENUM_TYPE_NOT_VALID = 0,

	};


	/// Texture formats:
	enum class ENUM_TEXTURE_FORMAT
	{
		None=0,
		BC1,         
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
		D16F,
		D24F,
		D32F,
		D0S8,

		Count
	};
	
    enum class ENUM_FRAMEBUFFER_TYPE
    {
        ENUM_TYPE_INVALID=0,
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

    enum class ENUM_PASS_TYPE 
    {
        ENUM_TYPE_INVALID=0,
        ENUM_PASS_COLOR,
        ENUM_PASS_DEPTH,
    };

    enum class ENUM_STATE_TYPE
    {

    };

} // namespace name
#endif // !_MXRENDER_ENUM_