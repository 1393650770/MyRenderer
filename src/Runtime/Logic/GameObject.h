#pragma once
#ifndef _GAMEOBJECT_
#define _GAMEOBJECT_
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include "Object.h"
#include "Component/TransformComponent.h"
#include "Component/StaticMeshComponent.h"

namespace MXRender { struct MeshObject; }

namespace MXRender { class Material; }


namespace MXRender
{

	class GameObject :public Object
	{
	private:
	protected:
		TransformComponent* transform=nullptr;
		StaticMeshComponent* staticmesh=nullptr;
		Material* material=nullptr;
		std::string name;
		std::vector<MeshObject*> sub_mesh;
	public:
		

		GameObject();
		GameObject(GameObject&& gameobject);
		GameObject(const std::string& name,const std::string& mesh_path,bool is_prefabs=false);
		virtual ~GameObject();
		StaticMeshComponent* get_staticmesh();
		TransformComponent* get_transform();
		std::string get_name();
		Material* get_material();
		void set_material(Material* in_material) ;

	};



}
#endif 
