#include "RenderInterface.h"
#include "Platform/PlatformWindow.h"
#include "RHI/RenderViewport.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)

void RenderInterface::OnInit(PlatformWindow* in_window, RHI::Viewport* in_viewport)
{
	OnInit_Logic(in_window, in_viewport);
	OnInit_Render();
}

void RenderInterface::OnShutdown()
{
	OnShutdown_Render();
	OnShutdown_Logic();
}

MYRENDERER_END_NAMESPACE
