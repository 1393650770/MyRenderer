#include "TransformComponent.h"
#include <stdexcept>
#include "glm/ext/matrix_transform.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/common.hpp>
#include <glm/gtx/quaternion.hpp>

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
	return glm::toMat4(glm::quat(rotation));
}


glm::mat4 MXRender::TransformComponent::get_translation_matrix()
{
	return  glm::translate(glm::mat4(1.0f), translation);
}

glm::mat4 MXRender::TransformComponent::get_scale_matrix()
{

	return glm::scale(glm::mat4(1.0f), scale);

}

glm::mat4 MXRender::TransformComponent::get_model_matrix()
{
	return get_relative_model_matrix();
}

glm::mat4 MXRender::TransformComponent::get_relative_model_matrix()
{
	return relative_model_matrix;
}

glm::vec3 MXRender::TransformComponent::get_translation()
{
	return translation;
}

glm::vec3 MXRender::TransformComponent::get_scale()
{
	return scale;
}

glm::vec3 MXRender::TransformComponent::get_rotation()
{
	return rotation;
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

std::string MXRender::TransformComponent::get_component_type_name()
{
	return "TransformComponent";
}
