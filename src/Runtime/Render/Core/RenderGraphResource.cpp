#include "RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

UInt32 RenderGraphResourceBase::GetID() CONST
{
	return id;
}

CONST String& RenderGraphResourceBase::GetName() CONST
{
	return name;
}

void RenderGraphResourceBase::SetName(CONST String& in_name)
{
	name = in_name;
}

Bool RenderGraphResourceBase::GetIsTransient() CONST
{
	return create_pass != nullptr;
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
