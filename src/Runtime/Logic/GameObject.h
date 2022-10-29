#pragma once
#ifndef _GAMEOBJECT_
#define _GAMEOBJECT_
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include "Object.h"
#include "Component/TransformComponent.h"
#include "Component/StaticMeshComponent.h"

namespace MXRender
{

	class GameObject :public Object
	{
	private:
	protected:
		TransformComponent* transform;
		StaticMeshComponent* staticmesh;
	public:
		
		GameObject();
		GameObject(GameObject&& gameobject);
		GameObject(const std::string& mesh_path);
		virtual ~GameObject();
		StaticMeshComponent* get_staticmesh();
		TransformComponent* get_transform();
	};

}
#endif 
