#pragma once
#include "Layer.h"
#include "Optimizer.h"
#include "RHI/RenderCommandList.h"

namespace MXNN {

using namespace MXRender::RHI;

// ============================================================
// SequentialModel — a stack of layers + loss function
// ============================================================
MYRENDERER_BEGIN_CLASS(SequentialModel)
#pragma region MATHOD
public:
	explicit SequentialModel(UInt32 in_max_batch_size);
	~SequentialModel();

	void AddLayer(UniquePtr<ILayer> in_layer);
	void SetOptimizer(UniquePtr<IOptimizer> in_opt);

	// One training step: zero grads → forward all → backward all → update all
	// Input: [active_batch_size x input_dim] raw float data on GPU
	// labels: per-sample class index [0..num_classes-1]
	// Returns: average loss for the batch
	Float32 TrainStep(CommandList* in_cmd, Tensor& in_input,
		CONST Vector<UInt8>& in_labels, UInt32 in_active_batch_size);

	// Inference only (no backward, no update)
	Vector<UInt8> Predict(CommandList* in_cmd, Tensor& in_input, UInt32 in_batch_size);
	// -- [AI] Split predict: record forward, then download after GPU flush
	void PredictForward(CommandList* in_cmd, Tensor& in_input);
	Vector<UInt8> GetPredictions(UInt32 in_batch_size);
	// -- [AI]
	void Save(CONST String& in_filepath);
	void Load(CONST String& in_filepath);
	void ClearTempSRBs();

	UInt32 MaxBatchSize() CONST { return max_batch_size_; }
protected:
private:
	void ZeroAllGradients(CommandList* in_cmd);
	void UpdateAllWeights(CommandList* in_cmd, Float32 in_inv_batch_size, UInt32 in_step);
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	Vector<UniquePtr<ILayer>> layers_;
	UniquePtr<IOptimizer> optimizer_;
	UInt32 max_batch_size_;
	UInt32 step_count_ = 0;

	RenderPipelineState* zero_grad_pipeline_ = nullptr;
	ShaderResourceBinding* zero_grad_srb_ = nullptr;
	Vector<ShaderResourceBinding*> zg_temp_srbs_; // temp SRBs, freed in dtor
	Tensor zg_pc_buf_;  // zero-grad param buffer
	Tensor label_buf_;  // GPU label buffer
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
