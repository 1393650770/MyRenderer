#include"TextureManager.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Input/InputSystem.h"
#include "../Logic/TaskScheduler.h"
#include "../RHI/Vulkan/VK_Texture.h"

namespace MXRender
{

	TextureManager::TextureManager()
	{

	}

	TextureManager::~TextureManager()
	{
		for (auto& it: texture_cache)
		{
			delete it.second;
		}
	}

	VK_Texture* TextureManager::get_or_create_texture(const std::string& name, ENUM_TEXTURE_TYPE texture_type, const std::string& path/*=""*/)
	{
		if (texture_cache.find(name)!=texture_cache.end())
		{
			return texture_cache[name];
		}
		else 
		{
			if (path=="")
			{
				return nullptr;
			}
			else
			{
				VK_Texture* texture=new VK_Texture(texture_type,path);
				texture_cache[name]=texture;
				return texture;
			}
		}
	}

}