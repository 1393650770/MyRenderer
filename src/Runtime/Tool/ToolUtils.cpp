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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE