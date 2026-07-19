#pragma once
#ifndef _COMPUTEUTILS_
#define _COMPUTEUTILS_
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
class RenderPipelineState;
class ShaderResourceBinding;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Tool)

MYRENDERER_BEGIN_CLASS(ComputeUtils)
#pragma region METHOD
public:
	// The `run_compute` lambda every compute sample re-implemented
	// (Fluid2D/Fluid3D once each, Ocean four times in one file):
	// bind pipeline + SRB, dispatch, then the write->read AND write->write
	// barriers that make back-to-back dispatches on the same resources safe.
	// The SRB must be fully bound at init time (never SetResource per frame).
	static void METHOD(DispatchWithBarrier)(RHI::CommandList* in_cmd, RHI::RenderPipelineState* in_pso, RHI::ShaderResourceBinding* in_srb, UInt32 in_group_x, UInt32 in_group_y = 1, UInt32 in_group_z = 1);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:

private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
