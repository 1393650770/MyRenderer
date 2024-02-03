#include "RenderRHI.h"
#include "Platform/Platform.h"

MXRender::RHI::RenderRHI* g_render_rhi = nullptr;

void RHIInit()
{
	if (g_render_rhi == nullptr)
	{
		g_render_rhi = PlatformCreateDynamicRHI();
	}
}

void RHIShutdown()
{

}

MXRender::RHI::Viewport* RHICreateViewport(void* window_handle, Int width, Int height, Bool is_full_screen)
{
	return g_render_rhi->CreateViewport(window_handle, width, height, is_full_screen);
}
