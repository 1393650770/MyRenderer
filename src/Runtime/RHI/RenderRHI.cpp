#include "RenderRHI.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

RenderRHI* g_render_rhi = nullptr;


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

extern CORE_API void RHIInit()
{
	if (MXRender::RHI::g_render_rhi == nullptr)
	{
		MXRender::RHI::g_render_rhi = PlatformCreateDynamicRHI();
	}
}
