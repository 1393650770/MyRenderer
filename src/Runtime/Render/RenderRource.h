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
		unsigned m_width{ 0 };
		unsigned m_height{ 0 };
		unsigned m_depth{ 0 };
		unsigned m_mip_levels{ 0 };
		unsigned m_array_layers{ 0 };
		void* m_pixels{ nullptr };

		ENUM_TEXTURE_FORMAT m_format{ ENUM_TEXTURE_FORMAT::Unknown };
		ENUM_TEXTURE_TYPE   m_type{ ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID };

		TextureData() = default;
		~TextureData()
		{
			if (m_pixels)
			{
				free(m_pixels);
			}
		}
		bool is_valid() const { return m_pixels != nullptr; }
	};
}


#endif _RENDERROURCE_