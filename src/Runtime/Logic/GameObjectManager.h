#pragma once
#ifndef _GAMEOBJECTMANAGER_
#define _GAMEOBJECTMANAGER_
#include <vector>
#include <string>


#include "GameObject.h"
#include "Camera/Camera.h"
#include <unordered_map>
#include "../RHI/Vulkan/VK_Utils.h"


namespace MXRender { class VK_Texture; }

namespace assets { struct PrefabInfo; }

namespace MXRender
{



	struct CacheTexture 
	{
		AllocatedImage image;
		VkImageView imageView;
	};

	struct PrefabObjectInfo 
	{
		std::unordered_map<uint64_t, GameObject*> node_objs;
		std::unordered_map<uint64_t, bool> node_objs_use;

	};


	class GameObjectManager 
	{
	friend GameObject;
	private:
	protected:
		std::unordered_map<std::string, MeshBase*> _meshes;
		std::unordered_map<std::string, CacheTexture> _loadedTextures;
		std::unordered_map<std::string, assets::PrefabInfo*> _prefabCache;
		bool load_image_to_cache(GraphicsContext* context,const char* name, const char* path);
		MeshBase* get_mesh(const std::string& name);		
		bool load_prefab(const std::string& name, const char* path, glm::mat4 root, GraphicsContext* context);
	public:
		std::vector<MeshObject> prefab_renderables;
		std::vector<GameObject> object_list;
		Camera main_camera;
		GameObjectManager(GraphicsContext* context);
		virtual ~GameObjectManager();
		void destroy_object_list(GraphicsContext* context);
		void start_load_prefabs(GraphicsContext* context);
		void set_overload_material(GraphicsContext* context);
		const std::unordered_map<std::string, MeshBase*>& get_mesh_cache() const;
		const std::unordered_map<std::string, CacheTexture >& get_texture_cache() const;
		const std::unordered_map<std::string, assets::PrefabInfo*>& get_prefab_cache() const;
	};

}
#endif 
