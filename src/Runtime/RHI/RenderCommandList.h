#pragma once


#ifndef _RENDERCOMMANDLIST_
#define _RENDERCOMMANDLIST_
#include "Core/ConstDefine.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class RenderPipelineState;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CommandList,public RenderResource)

#pragma region METHOD
public:
	CommandList() DEFAULT;
	VIRTUAL ~CommandList() DEFAULT;
	VIRTUAL void METHOD(Begin)() PURE;
	VIRTUAL void METHOD(End)() PURE;
	VIRTUAL void METHOD(SetPipeline)(RenderPipelineState* pipeline_state) PURE;
	VIRTUAL void METHOD(SetRenderTarget)(CONST Vector<Texture*>& render_targets, Texture* depth_stencil, CONST Vector<ClearValue>& clear_values, Bool has_dsv_clear_value) PURE;
private:

protected:
#pragma endregion

#pragma region MEMBER
public:

private:

protected:
#pragma endregion

MYRENDERER_END_CLASS



MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

