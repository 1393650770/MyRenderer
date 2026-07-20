#include<iostream>
#include "Application/Window.h"
#include "EditorRender/Render.h"

int main()
{
	MXRender::Application::Window window("MXRender - Render View");
	MXRender::Application::EditorRenderPipeline render(window.GetPlatformWindow());
	window.InitWindow();
	window.Run(&render);
	system("pause");

	return 0;
}
