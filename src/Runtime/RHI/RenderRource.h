#pragma once


#ifndef _RENDERROURCE_
#define _RENDERROURCE_
#include "../RHI/RenderEnum.h"
#include <cstdint>
#include <memory>
#include <string>
#include "../Core/BaseObject.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderResource,public IObject)
public:
	RenderResource() DEFAULT;
	VIRTUAL~RenderResource() DEFAULT;

private:

protected:

MYRENDERER_END_CLASS

class TextureData
{
public:
	unsigned width{ 0 };
	unsigned height{ 0 };
	unsigned depth{ 0 };
	unsigned mip_levels{ 0 };
	unsigned array_layers{ 0 };
	void* pixels{ nullptr };

	ENUM_TEXTURE_FORMAT format{ ENUM_TEXTURE_FORMAT::Unknown };
	ENUM_TEXTURE_TYPE   type{ ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID };

	TextureData() = default;
	~TextureData()
	{
		if (pixels)
		{
			free(pixels);
		}
	}
	bool is_valid() const { return pixels != nullptr; }
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE



	


#endif _RENDERROURCE_