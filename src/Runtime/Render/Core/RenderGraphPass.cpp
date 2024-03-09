#include "RenderGraphPass.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

CONST String& RenderGraphPassBase::GetName() CONST
{
	return pass_name;
}

void RenderGraphPassBase::SetName(CONST String& in_name)
{
	pass_name = in_name;
}

Bool RenderGraphPassBase::GetIsCullable() CONST
{
	return is_cullable;
}

void RenderGraphPassBase::SetIsCullable(Bool in_is_cullable)
{
	is_cullable = in_is_cullable;
}

RenderGraphPassBase::RenderGraphPassBase(CONST std::string& in_name):pass_name(in_name), is_cullable(false), ref_count(0)
{

}


RenderGraphPassBuilder::RenderGraphPassBuilder(RenderGraph* in_rendergraph, RenderGraphPassBase* in_render_pass) : graph(in_rendergraph), pass(in_render_pass)
{

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
