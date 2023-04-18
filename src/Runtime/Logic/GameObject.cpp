#include "GameObject.h"


void MXRender::GameObject::spawn_all_component()
{
	transform = new TransformComponent();
	staticmesh = new StaticMeshComponent();
	component_array.push_back(transform);
	component_array.push_back(staticmesh);
}

MXRender::GameObject::GameObject()
{
	spawn_all_component();
}

MXRender::GameObject::GameObject(const char* name)
{
	spawn_all_component();
	this->name=std::string(name);
}

MXRender::GameObject::GameObject(const std::string& name)
{
	spawn_all_component();
	this->name=name;
}

MXRender::GameObject::GameObject(GameObject&& gameobject)
{
	this->staticmesh=gameobject.staticmesh;
	this->transform=gameobject.transform;
	this->name= gameobject.name;
	this->parent_object=gameobject.parent_object;
	this->sub_objects.clear();
	this->sub_objects= gameobject.sub_objects;
	this->component_array= gameobject.component_array;
	gameobject.sub_objects.clear();
	gameobject.parent_object=nullptr;
	gameobject.staticmesh=nullptr;
	gameobject.transform=nullptr;
	gameobject.component_array.clear();
}

MXRender::GameObject::GameObject(const std::string& name, const std::string& mesh_path, bool is_prefabs)
{
	spawn_all_component();
	this->name = name;

	staticmesh->load_mesh(mesh_path);
	
}

MXRender::GameObject::~GameObject()
{
	if (transform)
	{
		delete transform;
	}
	if (staticmesh)
	{
		delete staticmesh;
	}
	//delete in materialsystem
	//if (material)
	//{
	//	delete material;
	//}
	for (auto& it:sub_objects)
	{
		delete it;
	}
}

MXRender::StaticMeshComponent* MXRender::GameObject::get_staticmesh()
{
	return staticmesh;
}

MXRender::TransformComponent* MXRender::GameObject::get_transform()
{	
	return transform;
}

std::string MXRender::GameObject::get_name()
{
	return name;
}

 MXRender::Material* MXRender::GameObject::get_material() const
{
	return staticmesh->get_material();
}

void MXRender::GameObject::set_material(Material* in_material)
{
	staticmesh->set_overmaterial( in_material);
}

