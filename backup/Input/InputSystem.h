#pragma once
#ifndef _INPUT_SYSTEM_
#define _INPUT_SYSTEM_
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>



struct GLFWwindow;
namespace MXRender { class Object; }
namespace MXRender { class GameObject; }




namespace MXRender
{
	enum class ENUM_INPUT_SYSTEM
	{
		NONE=0,
		WINDOWS,


		COUNT

	};

	enum ENUM_BUTTON_STATE:int
	{
		PRESS=0,
		REPEAT,
		RELEASE,
	};
	class InputSystem
	{
	private:
		
	protected:

	public:
		Object* cur_controller_object;
		InputSystem();
		virtual ~InputSystem();

		virtual void process_input(GLFWwindow* window);
	};

}
#endif 
