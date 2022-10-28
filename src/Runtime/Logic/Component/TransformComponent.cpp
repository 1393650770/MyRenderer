#include "TransformComponent.h"
#include <stdexcept>
#include "glm/ext/matrix_transform.hpp"


void MXRender::TransformComponent::update_relative_model_matrix()
{
	relative_model_matrix = get_translation_matrix() * get_rotation_matrix() * get_scale_matrix();
}

MXRender::TransformComponent::TransformComponent():rotation(glm::vec3(0.0f)),translation(glm::vec3(0.0f)),scale(glm::vec3(1.0f)),relative_model_matrix(glm::mat4(1.0f))
{
	component_type=ComponentType::TRANSFORM;

}

MXRender::TransformComponent::~TransformComponent()
{

}


void MXRender::TransformComponent::set_translation(glm::vec3 new_translation)
{
	translation=new_translation;
	update_relative_model_matrix();
}

void MXRender::TransformComponent::set_rotation(glm::vec3 new_rotation)
{
	rotation=new_rotation;
	update_relative_model_matrix();
}

void MXRender::TransformComponent::set_euler_rotation(glm::vec3 new_rotation)
{
	double k = std::acos(-1.0f) / 180.0f;
	rotation = rotation * static_cast<float>(k);
	update_relative_model_matrix();

}

void MXRender::TransformComponent::set_scale(glm::vec3 new_scale)
{
	scale=new_scale;
	update_relative_model_matrix();
}

void MXRender::TransformComponent::set_translation_rotation_scale(glm::vec3 new_translation, glm::vec3 new_rotation, glm::vec3 new_scale)
{
	translation = new_translation;
	rotation = new_rotation;
	scale = new_scale;
	update_relative_model_matrix();
}

glm::mat4 MXRender::TransformComponent::get_rotation_matrix()
{
	return glm::rotate(
	glm::rotate(
		glm::rotate(
			glm::mat4(1), 
			rotation.x, 
			{ 1, 0, 0 }
		), 
		rotation.y, 
		{ 0, 1, 0 }
		), 
	rotation.z, 
	{ 0, 0, 1 }
	);
}


glm::mat4 MXRender::TransformComponent::get_translation_matrix()
{
	return glm::mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		translation.x,translation.y,translation.z, 1
	);
}

glm::mat4 MXRender::TransformComponent::get_scale_matrix()
{
	return glm::mat4(
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	);
}

glm::mat4 MXRender::TransformComponent::get_model_matrix()
{
	return get_relative_model_matrix();
}

glm::mat4 MXRender::TransformComponent::get_relative_model_matrix()
{
	return relative_model_matrix;
}

void MXRender::TransformComponent::on_start()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::TransformComponent::on_update()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::TransformComponent::update(float delta_time)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::TransformComponent::on_end()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void MXRender::TransformComponent::on_destroy()
{
	throw std::logic_error("The method or operation is not implemented.");
}
