#pragma once


#ifndef _RENDERROURCE_
#define _RENDERROURCE_
#include "../RHI/RenderEnum.h"
#include <cstdint>
#include <memory>
#include <string>


namespace MXRender
{
	class RenderRource
	{

	};
	class TextureData
	{
	public:
		unsigned width{ 0 };
		unsigned height{ 0 };
		unsigned depth{ 0 };
		unsigned mip_levels{ 0 };
		unsigned array_layers{ 0 };
		void* pixels{ nullptr };
		bool is_free_by_me{ true };
		ENUM_TEXTURE_FORMAT format{ ENUM_TEXTURE_FORMAT::Unknown };
		ENUM_TEXTURE_TYPE   type{ ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID };

		TextureData() = default;
		~TextureData()
		{
			if (is_free_by_me &&pixels)
			{
				free(pixels);
			}
		}
		bool is_valid() const { return pixels != nullptr; }
	};
}


#endif _RENDERROURCE_