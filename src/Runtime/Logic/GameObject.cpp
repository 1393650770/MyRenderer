#include "GameObject.h"



MXRender::GameObject::GameObject()
{
	transform=new TransformComponent();
	staticmesh=new StaticMeshComponent();
	component_array.push_back(transform);
	component_array.push_back(staticmesh);
}

MXRender::GameObject::GameObject(GameObject&& gameobject)
{
	this->staticmesh=gameobject.staticmesh;
	this->transform=gameobject.transform;
	gameobject.staticmesh=nullptr;
	gameobject.transform=nullptr;
	gameobject.component_array.clear();
}

MXRender::GameObject::GameObject(const std::string& mesh_path)
{
	transform = new TransformComponent();
	staticmesh = new StaticMeshComponent();
	staticmesh->load_mesh(mesh_path);
}

MXRender::GameObject::~GameObject()
{
	delete transform;
	delete staticmesh;
}

MXRender::StaticMeshComponent* MXRender::GameObject::get_staticmesh()
{
	return staticmesh;
}

MXRender::TransformComponent* MXRender::GameObject::get_transform()
{	
	return transform;
}

MXRender::Material* MXRender::GameObject::get_material()
{
	return material;
}

void MXRender::GameObject::set_material(Material* in_material)
{
	material= in_material;
}

