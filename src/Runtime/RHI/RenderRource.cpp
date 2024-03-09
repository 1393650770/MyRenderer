#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

 void RenderResource::Release()
{
	if (--ref_count == 0)
		delete this;
}

UInt64 RenderGraphiPipelineStateDesc::GetHash() CONST
{
	return 0;
}

void RenderResource::AddRef()
{
	++ref_count;
}

void RenderResource::Realize() CONST
{
	CHECK_WITH_LOG(true, "Not implemented");
}

void RenderResource::DeRealize() CONST
{
	CHECK_WITH_LOG(true , "Not implemented");
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE