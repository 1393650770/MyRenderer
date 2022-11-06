#pragma once
#ifndef _INPUT_SYSTEM_
#define _INPUT_SYSTEM_
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>




namespace MXRender
{
	enum class ENUM_INPUT_SYSTEM
	{
		NONE=0,
		WINDOWS,


		COUNT

	};
	class InputSystem
	{
	private:
		
	protected:
		std::unordered_map<std::string,int> input_key_map;
		std::unordered_map<int,std::function<void()>> input_func_map;
	public:
		void bind_func(std::string name,const std::function<void()>& func);
		virtual void run(int key);
		InputSystem();
		virtual ~InputSystem();

	};

}
#endif 
