#include<iostream>
#include "Runtime/Application/Window.h"

int main()
{
	MXRender::Application::Window window;
	window.InitWindow();
	window.Run(nullptr);
	system("pause");

	return 0;
}
