#include "ToolUtils.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

ENUM_TEXTURE_FORMAT ToolUtils::TranslateGliFormatToEngineFormat(CONST gli::format& in_format)
{
	switch (in_format)
	{
		case gli::FORMAT_UNDEFINED:
		{
			return ENUM_TEXTURE_FORMAT::Unknown;
		}
		case gli::FORMAT_R8_SSCALED_PACK8:
		case gli::FORMAT_R8_SNORM_PACK8:
		{
			return ENUM_TEXTURE_FORMAT::R8S;
		}
		case gli::FORMAT_R8_SRGB_PACK8:
		{
			return ENUM_TEXTURE_FORMAT::R8;
		}
		case gli::FORMAT_R8_UNORM_PACK8:
		{
			return ENUM_TEXTURE_FORMAT::R8U;
		}
		case gli::FORMAT_RG8_SINT_PACK8:
		{
			return ENUM_TEXTURE_FORMAT::R8I;
		}
		case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		{
			return ENUM_TEXTURE_FORMAT::BC3;
		}
		case gli::FORMAT_RGBA16_SFLOAT_PACK16:
		{
			return ENUM_TEXTURE_FORMAT::RGBA16S;
		}
		default:
		{
			CHECK_WITH_LOG(true, "Error: Texture format is not supported");
		return ENUM_TEXTURE_FORMAT::Unknown;
	}
	}
}

// ---- Enum ↔ string conversion tables ----

static CONST Char* kTextureFormatNames[] = {
	"None","BC1","BC1A","BC2","BC3","BC4","BC5","BC6H","BC7",
	"ETC1","ETC2","ETC2A","ETC2A1","PTC12","PTC14","PTC12A","PTC14A","PTC22","PTC24",
	"ATC","ATCE","ATCI","ASTC4x4","ASTC5x5","ASTC6x6","ASTC8x5","ASTC8x6","ASTC10x5",
	"Unknown","R1","A8","R8","R8I","R8U","R8S","R16","R16I","R16U","R16F","R16S",
	"R32I","R32U","R32F","RG8","RG8I","RG8U","RG8S","RG16","RG16I","RG16U","RG16F","RG16S",
	"RG32I","RG32U","RG32F","RGB8","RGB8I","RGB8U","RGB8S","RGB9E5F",
	"RGB16I","RGB16U","RGB16F","RGB32I","RGB32U","RGB32F",
	"BGRA8","RGBA8","RGBA8I","RGBA8U","RGBA8S","RGBA16","RGBA16I","RGBA16U","RGBA16F","RGBA16S",
	"RGBA32I","RGBA32U","RGBA32F","R5G6B5","RGBA4","RGB5A1","RGB10A2","RG11B10F",
	"UnknownDepth","D16","D24","D24S8","D32","D32FS8","D16F","D24F","D32F","D0S8"
};
static constexpr Int kTextureFormatCount = sizeof(kTextureFormatNames) / sizeof(kTextureFormatNames[0]);

CONST Char* EnumToString(ENUM_TEXTURE_FORMAT format)
{
	Int idx = (Int)format;
	if (idx < 0 || idx >= kTextureFormatCount) return "RGBA8";
	return kTextureFormatNames[idx];
}

ENUM_TEXTURE_FORMAT StringToEnum_TextureFormat(CONST String& str)
{
	for (Int i = 0; i < kTextureFormatCount; ++i)
		if (str == kTextureFormatNames[i])
			return static_cast<ENUM_TEXTURE_FORMAT>(i);
	return ENUM_TEXTURE_FORMAT::None;
}

CONST Char* EnumToString(Render::RDGPassKind kind)
{
	switch (kind) {
	case Render::RDGPassKind::Graphics: return "Graphics";
	case Render::RDGPassKind::Compute:  return "Compute";
	case Render::RDGPassKind::Copy:     return "Copy";
	case Render::RDGPassKind::Custom:   return "Custom";
	default: return "Graphics";
	}
}

Render::RDGPassKind StringToEnum_PassKind(CONST String& str)
{
	if (str == "Compute") return Render::RDGPassKind::Compute;
	if (str == "Copy")    return Render::RDGPassKind::Copy;
	if (str == "Custom")  return Render::RDGPassKind::Custom;
	return Render::RDGPassKind::Graphics;
}

CONST Char* EnumToString(Render::RDGResourceKind kind)
{
	switch (kind) {
	case Render::RDGResourceKind::Texture:         return "Texture";
	case Render::RDGResourceKind::Buffer:          return "Buffer";
	case Render::RDGResourceKind::ExternalTexture: return "ExternalTexture";
	case Render::RDGResourceKind::DepthStencil:    return "DepthStencil";
	default: return "Texture";
	}
}

Render::RDGResourceKind StringToEnum_ResourceKind(CONST String& str)
{
	if (str == "Buffer")          return Render::RDGResourceKind::Buffer;
	if (str == "ExternalTexture") return Render::RDGResourceKind::ExternalTexture;
	if (str == "DepthStencil")    return Render::RDGResourceKind::DepthStencil;
	return Render::RDGResourceKind::Texture;
}

// -- 
static CONST Char* kBufferTypeNames[] = { "None","Static","Dynamic","Index","Vertex","Staging","Uniform","Storage","Indirect" };
static constexpr Int kBufferTypeCount = sizeof(kBufferTypeNames) / sizeof(kBufferTypeNames[0]);

CONST Char* EnumToString(ENUM_BUFFER_TYPE type)
{
	// Buffer types are bitmask flags; serialize primary value only
	for (Int i = 1; i < kBufferTypeCount; ++i)
		if ((UInt32)type == (1u << (i - 1))) return kBufferTypeNames[i];
	return kBufferTypeNames[0];
}

ENUM_BUFFER_TYPE StringToEnum_BufferType(CONST String& str)
{
	for (Int i = 1; i < kBufferTypeCount; ++i)
		if (str == kBufferTypeNames[i]) return static_cast<ENUM_BUFFER_TYPE>(1u << (i - 1));
	return ENUM_BUFFER_TYPE::None;
}

// -- 
static CONST Char* kTextureTypeNames[] = { "NotValid","2D","2D_MultiSample","2D_Array","2D_Depth","CubeMap","2D_Dynamic","3D" };
static constexpr Int kTextureTypeCount = sizeof(kTextureTypeNames) / sizeof(kTextureTypeNames[0]);

CONST Char* EnumToString(ENUM_TEXTURE_TYPE type) {
	Int idx = (Int)type;
	if (idx >= 0 && idx < kTextureTypeCount) return kTextureTypeNames[idx];
	return "2D";
}

ENUM_TEXTURE_TYPE StringToEnum_TextureType(CONST String& str) {
	for (Int i = 0; i < kTextureTypeCount; ++i)
		if (str == kTextureTypeNames[i]) return static_cast<ENUM_TEXTURE_TYPE>(i);
	return ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE