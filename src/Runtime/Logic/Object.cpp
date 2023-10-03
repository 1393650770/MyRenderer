#include "Object.h"
#include "Component/ComponentBase.h"



void MXRender::Object::add_component(ComponentBase* new_component)
{
	for (auto& component : component_array)
	{
		if (component->get_component_type_name() == new_component->get_component_type_name())
		{
			return;
		}
	}
	component_array.push_back(new_component);
	return ;
}

std::string MXRender::Object::get_name()
{
	return "null_implementation: get_name()";
}

MXRender::Object::Object()
{

}

MXRender::Object::~Object()
{

}

std::string MXRender::Object::to_string()
{
	return "";
}

void MXRender::Object::on_destroy()
{

}

void MXRender::Object::on_awake()
{

}

