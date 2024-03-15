#pragma once
#ifndef _MESHLOADER_
#define _MESHLOADER_
#include "Core/ConstDefine.h"
#include "RHI/RenderEnum.h"
#include "gli/format.hpp"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)
MYRENDERER_BEGIN_CLASS( ToolUtils )
public:
	static ENUM_TEXTURE_FORMAT METHOD(TranslateGliFormatToEngineFormat)(CONST gli::format& in_format);

MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif