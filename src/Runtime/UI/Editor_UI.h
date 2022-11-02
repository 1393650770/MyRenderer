#pragma once

#ifndef _EDITOR_UI_
#define _EDITOR_UI_



#include <memory>
#include "Window_UI.h"

namespace MXRender
{


	class EditorUI:public WindowUI
	{
	private:
	protected:
		void show_editor_UI();
	public:
		EditorUI();
		virtual ~EditorUI();
		virtual void initialize(WindowUIInitInfo* init_info) ;
		virtual void pre_render() ;
	};


}
#endif
