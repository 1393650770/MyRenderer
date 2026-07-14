#include "RenderInterface.h"

#include "RenderInterface.h"
#include "Application/Window.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)

//  Backward compat: OnInit calls split lifecycle methods
void RenderInterface::OnInit(Application::Window* window)
{
	OnInit_Logic(window);
	OnInit_Render();
}

//  Backward compat: OnShutdown calls split lifecycle methods
void RenderInterface::OnShutdown()
{
	OnShutdown_Render();
	OnShutdown_Logic();
}

MYRENDERER_END_NAMESPACE
