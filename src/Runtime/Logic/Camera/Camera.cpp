#include "Camera.h"
#include "../Component/TransformComponent.h"
#include "../Component/InputComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <functional>
#include <iostream>
#include "GLFW/glfw3.h"

void MXRender::Camera::calc_proj_mat()
{
	projection_mat = glm::perspective(glm::radians(fov), width/ height, near, far);
}

void MXRender::Camera::calc_otho_mat()
{
	projection_mat = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f);
}

void MXRender::Camera::camera_rotation(float delta_time)
{
	
	double xpos,ypos;
	glfwGetCursorPos(window, &xpos,&ypos);
	float xoffset = last_pos_x - xpos;
	float yoffset = last_pos_y - ypos; 
	//std::cout<<"camera_rotation :"<< xoffset<<"  "<< yoffset << std::endl;
	//update_rotation(xoffset*0.0000001f,yoffset * 0.0000001f);

	last_pos_x=xpos;
	last_pos_y=ypos;
}

void MXRender::Camera::camera_moveforward(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position.z += movement_speed * delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveback(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position.z -= movement_speed * delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveright(float delta_time)
{
	glm::vec3 position= transform->get_translation();
	position.x-=movement_speed* delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveleft(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position.x += movement_speed * delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::calc_projection_mat()
{
	switch (camera_type)
	{

	case MXRender::ENUM_CAMERA_TYPE::PROJECTION:
		calc_proj_mat();
		break;
	case MXRender::ENUM_CAMERA_TYPE::ORTHO:
		calc_otho_mat();
		break;
	default:
		calc_proj_mat();
		break;
	}
}

void MXRender::Camera::set_position(glm::vec3 new_position)
{
	transform->set_translation(new_position);
}

void MXRender::Camera::set_direction(glm::vec3 new_direction)
{
	direction = new_direction;
}

void MXRender::Camera::set_fov(float new_fov)
{
	fov = new_fov;
	calc_projection_mat();
}

void MXRender::Camera::set_far(float new_far)
{
	far = new_far;
	calc_projection_mat();
}

void MXRender::Camera::set_near(float new_near)
{
	near = new_near;
	calc_projection_mat();
}

void MXRender::Camera::set_speed(float new_speed)
{
	movement_speed = new_speed;
}

void MXRender::Camera::set_width(float new_width)
{
	width = new_width;
	calc_projection_mat();
}

void MXRender::Camera::set_height(float new_height)
{
	height = new_height;
	calc_projection_mat();
}

glm::vec3 MXRender::Camera::get_position() const
{
	return transform->get_translation();
}

glm::vec3 MXRender::Camera::get_direction() const
{
	return direction;
}

glm::mat4 MXRender::Camera::get_projection_mat() const
{
	return projection_mat;
}

glm::mat4 MXRender::Camera::get_view_mat() 
{
	view_mat= glm::lookAt(transform->get_translation(), transform->get_translation() + direction, up);
	return view_mat;
}

void MXRender::Camera::update_rotation(float x_offset, float y_offset)
{

	 glm::vec3 rotation= transform->get_rotation();
	 rotation.y = glm::clamp<float>(0.5f, rotation.y,x_offset);
	 rotation.x = glm::clamp<float>(0.5f, rotation.x, y_offset);

	if (rotation.x > 90.0f)
		rotation.x -= 90.0f;
	if (rotation.x < -90.0f)
		rotation.x += 90.0f;

	if (rotation.y > 90.0f)
		rotation.y -= 90.0f;
	if (rotation.y < -90.0f)
		rotation.y += 90.0f;

	glm::vec3 axis = glm::cross(direction, up); 
	glm::quat pitch_quat = glm::angleAxis(rotation.x, axis);

	axis = glm::cross(direction, axis);
	glm::quat yaw_quat = glm::angleAxis(rotation.y, up);

	glm::quat combined_rotation = pitch_quat * yaw_quat;

	direction = glm::rotate(combined_rotation, direction);

	direction = glm::normalize(direction);
	direction = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))); 
	up = glm::normalize(glm::cross(right, direction));

	transform->set_rotation(rotation);

}

void MXRender::Camera::set_window(GLFWwindow* new_window)
{
	window=new_window;
}

void MXRender::Camera::input_bingding_func()
{
	input_component->bind_func("Turn_right",PRESS,std::bind( &MXRender::Camera::Camera::camera_moveright ,this, std::placeholders::_1));
	input_component->bind_func("Turn_left", PRESS, std::bind(&MXRender::Camera::Camera::camera_moveleft, this, std::placeholders::_1));
	input_component->bind_func("Turn_forward", PRESS, std::bind(&MXRender::Camera::Camera::camera_moveforward, this, std::placeholders::_1));
	input_component->bind_func("Turn_back", PRESS, std::bind(&MXRender::Camera::Camera::camera_moveback, this, std::placeholders::_1));
	input_component->bind_func("Rotate", PRESS, std::bind(&MXRender::Camera::Camera::camera_rotation, this, std::placeholders::_1));
}


MXRender::Camera::Camera():fov(60.0f), far(1000.0f), near(0.1f), movement_speed(1.0f), width(1920.0f), height(1080.0f), direction(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), right(glm::normalize(glm::cross(up, direction))), focal_point(glm::vec3(0.0f, 0.0f, -1.0f))
{
	transform = new TransformComponent();
	input_component = new InputComponent();
	transform->set_translation(glm::vec3(0.0f, 0.0f, 5.0f));
	component_array.push_back(transform);
	component_array.push_back(input_component);
	calc_projection_mat();

	input_bingding_func();
}

MXRender::Camera::~Camera()
{
	delete transform;
	delete input_component;
}
