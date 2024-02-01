#pragma once
#ifndef _INPUTCOMPONENT_
#define _INPUTCOMPONENT_
#include <vector>
#include <string>
#include "ComponentBase.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include "../Input/InputSystem.h"

namespace MXRender
{

	class InputComponent:public ComponentBase
	{
	private:
		std::unordered_map<std::string, int> input_key_map;
		std::unordered_map<int, std::unordered_map<int, std::function<void(float)>>> input_func_map;
	public:
		void bind_func(std::string name, const ENUM_BUTTON_STATE& button_state, const std::function<void(float)>& func);
		virtual void run(int key, const ENUM_BUTTON_STATE& button_state);
		InputComponent();
		virtual ~InputComponent();


		virtual void on_start() override;


		virtual void on_update() override;


		virtual void update(float delta_time) override;


		virtual void on_end() override;


		virtual void on_destroy() override;


		virtual std::string get_component_type_name() override;

	};

}
#endif 
