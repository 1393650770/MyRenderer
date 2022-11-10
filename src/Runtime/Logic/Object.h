#pragma once
#ifndef _OBJECT_
#define _OBJECT_
#include <vector>
#include <string>

#include <glm/glm.hpp>

namespace MXRender { class ComponentBase; }

namespace MXRender
{

	class Object 
	{
	private:
	protected:
		std::vector<ComponentBase*> component_array;
	public:

		template<typename T>
		T* get_component(const std::string& component_type_name);

		Object();
		virtual ~Object();
		virtual std::string to_string();
		virtual void on_destroy();
		virtual void on_awake();
	};
	template<typename T>
	T* MXRender::Object::get_component(const std::string& component_type_name)
	{
		for (auto& component : component_array)
		{
			if (component->get_component_type_name() == component_type_name)
			{
				return static_cast<T*>(component);
			}
		}
		return nullptr;
	}
}
#endif 
