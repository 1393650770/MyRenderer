#include"RenderTexture.h"
#include<iostream>
#include <glad/glad.h>
#include <xutility>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
TextureDesc::TextureDesc(const TextureDesc& other)
{
    *this= other;
}

Texture::Texture(const TextureDesc& in_texture_desc):texture_desc(in_texture_desc)
{
    
}

TextureDesc Texture::GetTextureDesc() CONST
{
    return texture_desc;
}

void Texture::UpdateTextureData(CONST TextureDataPayload& texture_data_payload)
{

}


CONST Texture::TextureFormatAttribs& Texture::GetTextureFormatAttribs(ENUM_TEXTURE_FORMAT format)
{
	MYRENDERER_BEGIN_STRUCT(TextureFormatAttribsMap)
	public:
		TextureFormatAttribsMap()
		{
	#define INIT_TEX_FORMAT_INFO(TexFmt, ComponentSize, NumComponents, ComponentType, IsTypeless, BlockWidth, BlockHeight) \
					format_attribs_map[ (UInt32)TexFmt ] = TextureFormatAttribs{ TexFmt,ComponentType, NumComponents, ComponentSize , IsTypeless, BlockWidth, BlockHeight};
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::RGBA32F, 4, 4, ENUM_TEXTURE_COMPONENT_FORMAT::Float, false, 1, 1)
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::RGBA16F, 2, 4, ENUM_TEXTURE_COMPONENT_FORMAT::Float, false, 1, 1)
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::R32F, 4, 1, ENUM_TEXTURE_COMPONENT_FORMAT::Float, false, 1, 1)
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::RGBA32U, 4, 4, ENUM_TEXTURE_COMPONENT_FORMAT::UInt, false, 1, 1)
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::RGBA16U, 2, 4, ENUM_TEXTURE_COMPONENT_FORMAT::UInt, false, 1, 1)
			INIT_TEX_FORMAT_INFO(ENUM_TEXTURE_FORMAT::R32U, 4, 1, ENUM_TEXTURE_COMPONENT_FORMAT::UInt, false, 1, 1)
		}
	CONST TextureFormatAttribs& operator[](ENUM_TEXTURE_FORMAT format) CONST
		{
			if (format >= ENUM_TEXTURE_FORMAT::None && format < ENUM_TEXTURE_FORMAT::Count)
			{
				const auto& Attribs = format_attribs_map[(UInt32)format];
				return Attribs;
			}
			else
			{
				String error_msg = "Texture format (" + std::to_string((UInt32)format) + ") is out of allowed range [0, " + std::to_string((UInt32)ENUM_TEXTURE_FORMAT::Count - 1) + "]";
				CHECK_WITH_LOG(true, error_msg);
				return format_attribs_map[0];
			}
		}
	private:
		Array<TextureFormatAttribs, (UInt32)ENUM_TEXTURE_FORMAT::Count> format_attribs_map{};
	MYRENDERER_END_STRUCT

	static CONST TextureFormatAttribsMap s_texture_format_attribs_map;

	return s_texture_format_attribs_map[format];
}


TextureDataPayload::TextureDataPayload(CONST TextureDataPayload& other)
{
    *this = other;
}


TextureDataPayload::MipLevelProperties TextureDataPayload::GetMipLevelProperties(UInt8 in_mip_level) CONST
{
	MipLevelProperties mip_props;
	const auto& format_attribs = Texture::GetTextureFormatAttribs(format);

	mip_props.logic_width = max(width >> in_mip_level, 1u);
	mip_props.logic_height = max(height >> in_mip_level, 1u);
	mip_props.depth = max(depth >> in_mip_level, 1u);
	//TODO: support compressed formats
	//if (format_attribs.ComponentType == COMPONENT_TYPE_COMPRESSED)
	//{
	//}
	//else
	{
		mip_props.storage_height = mip_props.logic_height;
		mip_props.storage_width = mip_props.logic_width;
		mip_props.row_size = UInt64{ mip_props.storage_width } *UInt32{ format_attribs.component_count } *UInt32{ format_attribs.single_component_byte_size };
		mip_props.slice_size = mip_props.row_size * mip_props.storage_height;
		mip_props.mip_size = mip_props.slice_size * mip_props.depth;
	}

	return mip_props;
}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
