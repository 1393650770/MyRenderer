#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "Layer.h"
#include "Optimizer.h"
#include "RHI/RenderCommandList.h"

namespace MXNN {

using namespace MXRender::RHI;

// ============================================================
// SequentialModel — a stack of layers + loss function
// ============================================================
class SequentialModel {
public:
	SequentialModel(uint32_t max_batch_size);

	void AddLayer(std::unique_ptr<ILayer> layer);
	void SetOptimizer(std::unique_ptr<IOptimizer> opt);

	// One training step: zero grads → forward all → backward all → update all
	// Input: [active_batch_size x input_dim] raw float data on GPU
	// labels: per-sample class index [0..num_classes-1]
	// Returns: average loss for the batch
	float TrainStep(CommandList* cmd, Tensor& input,
					const std::vector<uint8_t>& labels, uint32_t active_batch_size);

	// Inference only (no backward, no update)
	std::vector<uint8_t> Predict(CommandList* cmd, Tensor& input, uint32_t batch_size);

	uint32_t MaxBatchSize() const { return max_batch_size_; }

private:
	void ZeroAllGradients(CommandList* cmd);
	void UpdateAllWeights(CommandList* cmd, float inv_batch_size, uint32_t step);

	std::vector<std::unique_ptr<ILayer>> layers_;
	std::unique_ptr<IOptimizer> optimizer_;
	uint32_t max_batch_size_;
	uint32_t step_count_ = 0;

	RenderPipelineState* zero_grad_pipeline_ = nullptr;
	ShaderResourceBinding* zero_grad_srb_ = nullptr;
	Tensor zg_pc_buf_; // zero-grad param buffer
	Tensor label_buf_;
};

} // namespace MXNN
