#pragma once
#include "Tensor.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {

using namespace MXRender::RHI;

// ============================================================
// IOptimizer — abstract weight update interface
// ============================================================
MYRENDERER_BEGIN_CLASS(IOptimizer)
#pragma region MATHOD
public:
	IOptimizer() MYDEFAULT;
	VIRTUAL ~IOptimizer() MYDEFAULT;

	VIRTUAL void METHOD(Update)(CommandList* in_cmd, Tensor& in_params, Tensor& in_grads,
		Tensor& in_velocity, Float32 in_inv_batch_size, UInt32 in_step) PURE;
protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion
MYRENDERER_END_CLASS

// ============================================================
// SGD with Momentum
// v = momentum * v + (grad * inv_bs + wd * param)
// param -= lr * v
// ============================================================
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(SGD, public MXNN::IOptimizer)
#pragma region MATHOD
public:
	SGD(Float32 in_lr = 0.01f, Float32 in_momentum = 0.9f, Float32 in_weight_decay = 0.0f);
	~SGD();

	VIRTUAL void Update(CommandList* in_cmd, Tensor& in_params, Tensor& in_grads,
		Tensor& in_velocity, Float32 in_inv_batch_size, UInt32 in_step) OVERRIDE FINAL;

	Float32 LR() CONST { return lr_; }
protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	Float32 lr_, momentum_, weight_decay_;
	Tensor pc_buf_;
	RenderPipelineState* update_pipeline_ = nullptr;
	ShaderResourceBinding* update_srb_ = nullptr;
		Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
