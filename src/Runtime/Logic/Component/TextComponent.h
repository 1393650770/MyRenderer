#pragma once
#ifndef _TEXTCOMPONENT_
#define _TEXTCOMPONENT_
#include <vector>
#include <string>
#include "ComponentBase.h"
#include <glm/glm.hpp>

namespace MXRender { class TextBase; }

namespace MXRender
{
	class TextComponent:public ComponentBase
	{
	private:
	protected:
		TextBase* text_data=nullptr;
		std::string text_content;
	public:
		TextComponent();
		TextComponent(const std::string& ttf_path);
		virtual ~TextComponent();

		virtual void on_start() override;

		virtual void on_update() override;

		virtual void update(float delta_time) override;

		virtual void on_end() override;

		virtual void on_destroy() override;

		virtual std::string get_component_type_name() override;
		void reset_text(TextBase* in_text_data);
		void reset_text_content(const std::string& in_text_content);
		void render_text(RenderInfo* render_mesh_info);
		void bind_text(BindInfo* bind_mesh_info);
		TextBase* get_text_data();
	};

}
#endif 
