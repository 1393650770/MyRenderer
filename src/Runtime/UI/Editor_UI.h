#pragma once

#ifndef _EDITOR_UI_
#define _EDITOR_UI_



#include <memory>
#include "Window_UI.h"

namespace MXRender { class GraphicsContext; }

namespace MXRender
{

	struct EditorUIInitInfo:public WindowUIInitInfo
	{
	public: 
		GraphicsContext* context;
	};

	class EditorUI:public WindowUI
	{
	private:
	protected:
		GraphicsContext* context;
		void show_camera_detail();

	public:
		EditorUI();
		virtual ~EditorUI();
		virtual void initialize(WindowUIInitInfo* init_info) ;
		virtual void pre_render() ;
		void show_editor_left_ui();
		void show_editor_right_ui();
		void show_editor_top_ui();
		void show_editor_down_ui();
		void show_center_main_window();
	};


}
#endif
