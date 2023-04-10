#pragma once
#ifndef _TEXTUREMANAGER_
#define _TEXTUREMANAGER_
#include <memory>
#include <string>
#include <unordered_map>
#include "../RHI/Vulkan/VK_Texture.h"


namespace MXRender
{
	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();
		VK_Texture* get_or_create_texture(const std::string& name,ENUM_TEXTURE_TYPE texture_type=ENUM_TEXTURE_TYPE::ENUM_TYPE_NOT_VALID, const std::string& path="");
	private:
		std::unordered_map<std::string , VK_Texture* > texture_cache;
	};



}
#endif //_TEXTUREMANAGER_