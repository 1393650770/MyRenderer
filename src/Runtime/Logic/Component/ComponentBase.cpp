#include "ComponentBase.h"


MXRender::ComponentBase::ComponentBase()
{

}

MXRender::ComponentBase::~ComponentBase()
{

}

MXRender::ComponentBase::ComponentBase(ComponentType cur_component_type):component_type(cur_component_type)
{

}

bool MXRender::ComponentBase::get_active()
{
	return active;
}

void MXRender::ComponentBase::set_active(bool active)
{
	this->active=active;
}

MXRender::ComponentType MXRender::ComponentBase::get_component_type()
{
	return component_type;
}

void MXRender::ComponentBase::on_start()
{

}

void MXRender::ComponentBase::on_update()
{

}

void MXRender::ComponentBase::update(float delta_time)
{

}

void MXRender::ComponentBase::on_end()
{

}

void MXRender::ComponentBase::on_destroy()
{

}
