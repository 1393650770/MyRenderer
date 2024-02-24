#pragma once
#ifndef _RENDERPIPELINESTATE_
#define _RENDERPIPELINESTATE_
#include "RenderEnum.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderPipelineState, public RenderResource)
#pragma region METHOD
public:
	RenderPipelineState() DEFAULT;
	RenderPipelineState(CONST RenderGraphiPipelineStateDesc& in_desc);
	VIRTUAL ~RenderPipelineState();

protected:
private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	RenderGraphiPipelineStateDesc desc;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif //_RENDERSTATE_
