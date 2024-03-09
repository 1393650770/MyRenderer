#pragma once

#ifndef _PLATFORM_
#define _PLATFORM_
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI) 
class RenderRHI;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE


extern CORE_API MXRender::RHI::RenderRHI* PlatformCreateDynamicRHI();


#endif 
