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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE