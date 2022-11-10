#include<iostream>
#include "Runtime/Render/Window.h"
#include "Runtime/Render/DeferRender.h"


int main()
{

	std::shared_ptr<MXRender::DeferRender> defer_render = std::make_shared<MXRender::DeferRender>();

	MXRender::Window my_window;

	my_window.run(defer_render);

	//system("pause");

	return 0;
}
