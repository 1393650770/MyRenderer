#include "ComputeUtils.h"
#include "RHI/RenderEnum.h"
#include "RHI/RenderCommandList.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

void ComputeUtils::DispatchWithBarrier(RHI::CommandList* in_cmd, RHI::RenderPipelineState* in_pso, RHI::ShaderResourceBinding* in_srb, UInt32 in_group_x, UInt32 in_group_y, UInt32 in_group_z)
{
	in_cmd->SetComputePipeline(in_pso);
	in_cmd->SetShaderResourceBinding(in_srb);
	in_cmd->Dispatch(in_group_x, in_group_y, in_group_z);
	in_cmd->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::ShaderResource);
	in_cmd->ResourceBarrier(ENUM_RESOURCE_STATE::UnorderedAccess, ENUM_RESOURCE_STATE::UnorderedAccess);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
