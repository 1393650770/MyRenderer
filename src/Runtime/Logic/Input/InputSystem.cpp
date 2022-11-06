#include "InputSystem.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

void MXRender::InputSystem::bind_func(std::string name, const std::function<void()>& func)
{
	if (input_key_map.contains(name) == false)
	{
		throw std::logic_error("input.setting.ini dont count this name "+name);
	}
	input_func_map[input_key_map[name]]=func;
}

void MXRender::InputSystem::run(int key)
{
	if (input_func_map.contains(key) == false)
	{
		return;
	}
	input_func_map[key]();
}

MXRender::InputSystem::InputSystem()
{
	std::ifstream  in_fstream("Setting/input_setting.ini");
	in_fstream.open("Setting/input_setting.ini",std::ios_base::in);
	if (in_fstream.is_open())
	{
		std::string content;
		while (std::getline(in_fstream, content))
		{
			int n= content.find_first_of(" ");
			std::string name= content.substr(0,n);
			std::string key_string=content.substr(n);

			int key_num=atoi(key_string.c_str());

			input_key_map[name]=key_num;

		}
	
		in_fstream.close();
	}
}

MXRender::InputSystem::~InputSystem()
{

}
