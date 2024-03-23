#include<iostream>
#include "Application/Window.h"
#include "EditorRender/Render.h"
#include "GenCode/Schema/FlatbufferGen/bbbb_generated.h"
int main()
{
	MXRender::Application::Window window;
	MXRender::Application::EditorRenderPipeline render(&window);
	window.InitWindow();
	window.Run(&render);
	system("pause");

	return 0;
}
