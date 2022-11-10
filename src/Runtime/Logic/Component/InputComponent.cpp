#include "InputComponent.h"
#include <stdexcept>
#include "glm/ext/matrix_transform.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
#include "GLFW/glfw3.h"
#include <iostream>
#include <fstream>
#include "../../Render/DefaultSetting.h"
#include "../../Utils/Singleton.h"

void MXRender::InputComponent::bind_func(std::string name, const ENUM_BUTTON_STATE& button_state, const std::function<void(float)>& func)
{
	if (input_key_map.contains(name) == false)
	{
		throw std::logic_error("input.setting.ini dont count this name " + name);
	}
	input_func_map[input_key_map[name]][button_state] = func;
}

void MXRender::InputComponent::run(int key, const ENUM_BUTTON_STATE& button_state)
{
	if (input_func_map.contains(key) == false)
	{
		return;
	}
	input_func_map[key][button_state](0.02f);
}

MXRender::InputComponent::InputComponent()
{
	input_key_map["Turn_right"] = GLFW_KEY_D;
	input_key_map["Turn_left"] = GLFW_KEY_A;
	input_key_map["Turn_forward"] = GLFW_KEY_W;
	input_key_map["Turn_back"] = GLFW_KEY_S;
	input_key_map["Rotate"] = GLFW_MOUSE_BUTTON_LEFT;
	std::ifstream  in_fstream("Setting/input_setting.ini");
	in_fstream.open("Setting/input_setting.ini", std::ios_base::in);
	if (in_fstream.is_open())
	{
		std::string content;
		while (std::getline(in_fstream, content))
		{
			int n = content.find_first_of(" ");
			std::string name = content.substr(0, n);
			std::string key_string = content.substr(n);

			int key_num = atoi(key_string.c_str());

			input_key_map[name] = key_num;

		}

		in_fstream.close();
	}
}

MXRender::InputComponent::~InputComponent()
{

}

void MXRender::InputComponent::on_start()
{

}

void MXRender::InputComponent::on_update()
{

}

void MXRender::InputComponent::update(float delta_time)
{

}

void MXRender::InputComponent::on_end()
{

}

void MXRender::InputComponent::on_destroy()
{

}

std::string MXRender::InputComponent::get_component_type_name()
{
	return "InputComponent";
}
