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
		template<typename T>
		const T* get_component_const(const std::string& component_type_name) const;
		virtual std::string get_name();
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
			std::string name= component->get_component_type_name();
			if (name == component_type_name)
			{
				return static_cast<T*>(component);
			}
		}
		return nullptr;
	}
		
	template<typename T>
	const T* MXRender::Object::get_component_const(const std::string& component_type_name) const
	{
		for (auto& component : component_array)
		{
			if (component->get_component_type_name() == component_type_name)
			{
				return static_cast<const T*>(component);
			}
		}
		return nullptr;
	}
#define get_component_const(COMPONENT_TYPE) get_component_const<const COMPONENT_TYPE>(#COMPONENT_TYPE)
#define get_component(COMPONENT_TYPE) get_component<COMPONENT_TYPE>(#COMPONENT_TYPE)
}
#endif 
