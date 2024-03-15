#include "TextureLoader.h"
#include <gli/gli.hpp>
#include "ToolUtils.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

void TextureLoader::LoadTextureData(CONST String& filename, RHI::TextureDataPayload* data, std::atomic_bool& result)
{
	gli::texture texture=gli::load(filename);
	if (texture.empty()) 
		return;
	
	data->data.resize(texture.size());
	memcpy(data->data.data(), texture.data(), texture.size());
	data->width = texture.extent().x;
	data->height = texture.extent().y;
	data->depth = texture.extent().z;
	data->mip_level = texture.levels();
	data->layer_count = texture.layers();
	data->format = ToolUtils::TranslateGliFormatToEngineFormat(texture.format());
	switch (texture.target())
	{
		case gli::TARGET_2D:
		{
			data->type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
			data->depth = 1;
			break;
		}
		case gli::TARGET_2D_ARRAY:
		{
			data->type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_ARRAY;
			break;
		}
		case gli::TARGET_3D:
		{
			data->type = ENUM_TEXTURE_TYPE::ENUM_TYPE_3D;
			break;
		}
		case gli::TARGET_CUBE:
		{
			data->type = ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
			break;
		}
		default:
		{
			CHECK_WITH_LOG(true, "Loader Error: Texture target is not supported");
			break;
		}
	}
	result = true;
}

RHI::TextureDataPayload TextureLoader::LoadTextureData(CONST String& filename)
{
	RHI::TextureDataPayload texture_data_payload;
	gli::texture texture(gli::load(filename.c_str()));
	if (texture.empty())
		return texture_data_payload;
	texture_data_payload.data.resize(texture.size());
	memcpy(texture_data_payload.data.data(), texture.data(), texture.size());
	texture_data_payload.width = texture.extent().x;
	texture_data_payload.height = texture.extent().y;
	texture_data_payload.depth = texture.extent().z;
	texture_data_payload.mip_level = texture.levels();
	texture_data_payload.layer_count = texture.layers();
	texture_data_payload.format = ToolUtils::TranslateGliFormatToEngineFormat(texture.format());
	switch (texture.target())
	{
		case gli::TARGET_2D:
		{
			gli::texture2d texture2d(texture);
			texture_data_payload.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
			texture_data_payload.depth = 1;
			break;
		}
		case gli::TARGET_2D_ARRAY:
		{
			texture_data_payload.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D_ARRAY;
			break;
		}
		case gli::TARGET_3D:
		{
			texture_data_payload.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_3D;
			break;
		}
		case gli::TARGET_CUBE:
		{
			texture_data_payload.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP;
			break;
		}
		default:
		{
			CHECK_WITH_LOG(true, "Loader Error: Texture target is not supported");
			break;
		}
	}
	return texture_data_payload;
}
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE