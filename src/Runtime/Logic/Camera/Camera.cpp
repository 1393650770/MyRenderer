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

void MXRender::Camera::camera_rotation(Camera* camera,float delta_time)
{
	if (camera)
	{

		double xpos,ypos;
		glfwGetCursorPos(window, &xpos,&ypos);
		//std::cout << "camera_position :" << xpos << "  " << ypos <<" last_pos_x : "<< camera->last_pos_x<<"  "<< camera->last_pos_y << std::endl;
		if (camera->is_first_mouse_press)
		{
			camera->last_pos_x = xpos;
			camera->last_pos_y = ypos;
			camera->is_first_mouse_press = false;
		}
		float x_offset = camera->last_pos_x - xpos;
		float y_offset = camera->last_pos_y - ypos;
		//std::cout<<"camera_rotation :"<< x_offset<<"  "<< y_offset << std::endl;
		update_rotation(x_offset,y_offset);

		camera->last_pos_x=xpos;
		camera->last_pos_y=ypos;
	}
}

void MXRender::Camera::camera_rotation_relese(Camera* camera, float)
{
	if (camera)
	{
		camera->is_first_mouse_press = true;
	}
}

void MXRender::Camera::camera_moveforward(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position +=direction* movement_speed * delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveback(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position -=direction* movement_speed * delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveright(float delta_time)
{
	glm::vec3 position= transform->get_translation();
	position +=  right *movement_speed* delta_time;
	transform->set_translation(position);
}

void MXRender::Camera::camera_moveleft(float delta_time)
{
	glm::vec3 position = transform->get_translation();
	position -= right* movement_speed * delta_time;
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

glm::vec3 MXRender::Camera::get_right() const
{
	return right;
}

glm::vec3 MXRender::Camera::get_up() const
{
	return up;
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

float& MXRender::Camera::get_can_change_move_speed()
{
	return movement_speed;
}

float& MXRender::Camera::get_can_change_fov()
{
	return fov;
}

float& MXRender::Camera::get_can_change_near_plane()
{
	return near;
}

float& MXRender::Camera::get_can_change_far_plane()
{
	return far;
}

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest)
{
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f) {
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(180.0f, rotationAxis);
	}

	rotationAxis = cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);

}

void MXRender::Camera::update_rotation(float x_offset, float y_offset)
{

	 glm::vec3 rotation= transform->get_rotation();
	 x_offset = x_offset< -0.001f?-1.0f: x_offset > 0.001f? 1.0f:0.0f ;
	 y_offset = y_offset < -0.001f ? -1.0f : y_offset > 0.001f ? 1.0f : 0.0f;

	 if (rotation.x > 89.0f)
		 rotation.x = 89.0f;
	 if (rotation.x < -89.0f)
		 rotation.x = -89.0f;
	 glm::vec3  ViewDest = direction + right * x_offset * 0.008f * movement_speed + up * y_offset * 0.008f * movement_speed;

	 glm::quat rot1 = RotationBetweenVectors(direction, ViewDest);

	 direction = glm::normalize(glm::rotate(rot1, direction));

	 right = glm::normalize(glm::cross(direction, up));

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
	input_component->bind_func("Rotate_press", PRESS, std::bind(&MXRender::Camera::Camera::camera_rotation, this,this, std::placeholders::_1));
	input_component->bind_func("Rotate_relese",RELEASE, std::bind(&MXRender::Camera::Camera::camera_rotation_relese, this, this, std::placeholders::_1));
}


MXRender::Camera::Camera():fov(60.0f), far(100.0f), near(1.f), movement_speed(1.0f), width(1920.0f), height(1080.0f), direction(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 5.0f, 0.0f)), right(glm::normalize(glm::cross(up, direction))), focal_point(glm::vec3(0.0f, 0.0f, -1.0f))
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
