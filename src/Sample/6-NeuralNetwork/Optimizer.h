#pragma once
#include <cstdint>
#include "Tensor.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {

using namespace MXRender::RHI;

// ============================================================
// IOptimizer — abstract weight update interface
// ============================================================
struct IOptimizer {
	virtual ~IOptimizer() = default;
	virtual void Update(CommandList* cmd, Tensor& params, Tensor& grads,
						Tensor& velocity, float inv_batch_size, uint32_t step) = 0;
};

// ============================================================
// SGD with Momentum
// v = momentum * v + (grad * inv_bs + wd * param)
// param -= lr * v
// ============================================================
class SGD : public IOptimizer {
public:
	SGD(float lr = 0.01f, float momentum = 0.9f, float weight_decay = 0.0f);
	~SGD() override;

	void Update(CommandList* cmd, Tensor& params, Tensor& grads,
				Tensor& velocity, float inv_batch_size, uint32_t step) override;

	float LR() const { return lr_; }

private:
	float lr_, momentum_, weight_decay_;
	Tensor pc_buf_;
	RenderPipelineState* update_pipeline_ = nullptr;
	ShaderResourceBinding* update_srb_ = nullptr;
};

} // namespace MXNN
