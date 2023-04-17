#pragma once

#ifndef _WINDOW_UI_
#define _WINDOW_UI_



#include <memory>

namespace MXRender
{

	struct WindowUIInitInfo
	{

	};

	class WindowUI
	{
	public:
		WindowUI();
		virtual ~WindowUI();
		virtual void initialize(WindowUIInitInfo* init_info) = 0;
		virtual void initialize_resource()=0;
		virtual void pre_render() = 0;
	};


}
#endif
