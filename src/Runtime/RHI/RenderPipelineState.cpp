#include "RenderPipelineState.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)



RenderPipelineState::RenderPipelineState(CONST RenderGraphiPipelineStateDesc& in_desc):
	desc(in_desc)
{
}

RenderPipelineState::~RenderPipelineState()
{

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

