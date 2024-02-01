#pragma once
#ifndef _GAMEOBJECT_
#define _GAMEOBJECT_
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include "Object.h"
#include "Component/TransformComponent.h"
#include "Component/StaticMeshComponent.h"

namespace MXRender { class MaterialComponent; }

namespace MXRender { struct MeshObject; }

namespace MXRender { class Material; }


namespace MXRender
{

	class GameObject :public Object
	{
	private:
	protected:
		void spawn_all_component();

		TransformComponent* transform=nullptr;
		StaticMeshComponent* staticmesh=nullptr;
		std::string name;

	public:
		std::vector<GameObject*> sub_objects;
		GameObject* parent_object=nullptr;

		GameObject();
		GameObject(const std::string& name);
		GameObject(const char* name);
		GameObject(GameObject&& gameobject);
		GameObject(const std::string& name,const std::string& mesh_path,bool is_prefabs=false);
		virtual ~GameObject();
		StaticMeshComponent* get_staticmesh();
		TransformComponent* get_transform();
		std::string get_name() override;
		Material* get_material() const;
		void set_material(Material* in_material) ;

	};



}
#endif 
