#pragma once
#ifndef _COMPONENTBASE_s
#define _COMPONENTBASE_
#include <vector>
#include <string>


namespace MXRender
{

	enum class ComponentType
	{
		NONE = 0,
		TRANSFORM,
		STATICMESH,
		COUNT

	};
    class ComponentBase
    {
    private:
    protected:
		bool active=true;
		bool never_started = false;
		bool never_awaked = false;
		ComponentType component_type= ComponentType::NONE;
		ComponentBase(const ComponentBase&) = delete;
		ComponentBase& operator=(const ComponentBase&) = delete;
		ComponentBase(ComponentBase&&) = delete;
		ComponentBase& operator=(ComponentBase&&) = delete;
    public:

		ComponentBase();
		ComponentBase(ComponentType cur_component_type);
		virtual ~ComponentBase();
		bool get_active();
		void set_active(bool active);
		ComponentType get_component_type();
		virtual std::string get_component_type_name()=0;
        virtual void on_start();
		virtual void on_update();
        virtual void update(float delta_time);
        virtual void on_end();
		virtual void on_destroy();
    };

}
#endif 
