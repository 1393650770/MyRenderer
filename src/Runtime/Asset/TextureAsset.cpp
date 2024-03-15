#include "TextureAsset.h"
#include "Tool/TextureLoader.h"
#include <future>
#include "RHI/RenderRHI.h"
#include "RHI/RenderTexture.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Asset)


void TextureAsset::LoadTexture(CONST String& path)
{
	data = new RHI::TextureDataPayload();
	auto func = [&,this, path]()
	{
		Tool::TextureLoader::LoadTextureData(path, (this->data), this->is_loaded);
	};
	std::async(std::launch::async, func);
}

RHI::Texture* TextureAsset::GetTexture()
{
	if (texture != nullptr)
	{
		return texture;
	}
	else if (is_loaded)
	{
		if (texture == nullptr)
		{
			RHI::TextureDesc desc;
			desc.format = data->format;
			desc.width = data->width;
			desc.height = data->height;
			desc.depth = data->depth;
			desc.mip_level = data->mip_level;
			desc.layer_count = data->layer_count;
			desc.type = data->type;
			desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
			texture = RHICreateTexture(desc);
			texture->UpdateTextureData(*data);
			delete data;
			data = nullptr;
		}
		return texture;
	}
}

TextureAsset::~TextureAsset()
{
	if (texture != nullptr)
	{
		delete texture;
		texture = nullptr;
	}
	if (data != nullptr)
	{
		delete data;
		data = nullptr;
	}

}

TextureAsset::TextureAsset(CONST String& path)
{
	LoadTexture(path);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE