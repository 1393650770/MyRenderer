#pragma once


#ifndef _RENDERCOMMANDLIST_
#define _RENDERCOMMANDLIST_
#include "Core/ConstDefine.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_STRUCT(DrawAttribute)
public:
	UInt32                                    vertexCount = 0;
	UInt32                                    instanceCount = 0;
	UInt32                                    firstVertex = 0;
	UInt32                                    firstInstance = 0;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_NAMESPACE(RHI)
class RenderPipelineState;
class ShaderResourceBinding;
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CommandList,public RenderResource)

#pragma region METHOD
public:
	CommandList() MYDEFAULT;
	VIRTUAL ~CommandList() MYDEFAULT;
	VIRTUAL void METHOD(SetGraphicsPipeline)(RenderPipelineState* pipeline_state) PURE;
	VIRTUAL void METHOD(SetRenderTarget)(CONST Vector<Texture*>& render_targets, Texture* depth_stencil, CONST Vector<ClearValue>& clear_values, Bool has_dsv_clear_value) PURE;
	VIRTUAL void METHOD(SetShaderResourceBinding)(ShaderResourceBinding* srb) PURE;
	VIRTUAL void METHOD(Draw)(CONST DrawAttribute& draw_attr) PURE;
	VIRTUAL void METHOD(TransitionTextureState)(Texture* texture, CONST ENUM_RESOURCE_STATE& required_state) PURE;
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

