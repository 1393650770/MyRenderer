#pragma once
#ifndef _OBJECT_
#define _OBJECT_
#include <vector>
#include <string>

#include <glm/glm.hpp>

namespace MXRender
{

	class Object 
	{
	private:
	protected:
		
	public:
		Object();
		virtual ~Object();
		virtual std::string to_string();
		virtual void on_destroy();
		virtual void on_awake();
	};

}
#endif 
