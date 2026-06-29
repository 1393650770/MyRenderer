#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "Tensor.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {
using namespace MXRender::RHI;

// Param buffer layouts (must match shader binding 10)
struct FwdParams   { float in_dim, out_dim, max_batch_size, active_batch_size, has_relu; };
struct LossParams  { float in_dim, out_dim, max_batch_size, active_batch_size; };
struct BwdParams   { float in_dim, out_dim, max_batch_size, active_batch_size, has_relu; };
struct SgdParams   { float lr, momentum, wd, inv_bs, n_elem; };
struct ZeroParams  { float num_elements; };

struct IOptimizer;
struct ILayer {
	virtual ~ILayer() = default;
	virtual void Forward(CommandList*, Tensor& input) = 0;
	virtual void Backward(CommandList*, const Tensor& dL_dout, const Tensor& input_act) = 0;
	virtual void ZeroGradients(CommandList*) = 0;
	virtual void UpdateWeights(CommandList*, IOptimizer&, float inv_bs, uint32_t step) = 0;
	virtual Tensor& GetOutput() = 0;
	virtual Tensor& GetInputGradient() = 0;
	virtual std::vector<std::tuple<Tensor*, Tensor*, Tensor*>> GetParamTriples() = 0;
};

class LinearLayer : public ILayer {
public:
	LinearLayer(uint32_t in_dim, uint32_t out_dim, uint32_t max_batch_size, bool has_relu);
	~LinearLayer() override;
	void Forward(CommandList*, Tensor& input) override;
	void Backward(CommandList*, const Tensor& dL_dout, const Tensor& input_act) override;
	void ZeroGradients(CommandList*) override;
	void UpdateWeights(CommandList*, IOptimizer&, float inv_bs, uint32_t step) override;
	Tensor& GetOutput() override { return output_; }
	Tensor& GetInputGradient() override { return dL_dx_; }
	std::vector<std::tuple<Tensor*, Tensor*, Tensor*>> GetParamTriples() override;
private:
	void CreatePipelineAndSRB();
	uint32_t in_dim_, out_dim_, max_batch_size_; bool has_relu_;
	Tensor weight_, bias_, grad_w_, grad_b_, v_w_, v_b_;
	Tensor z_preact_, output_, dL_dx_;
	Tensor pc_buf_; // params for binding 10
	RenderPipelineState *fwd_pipeline_=nullptr, *bwd_pipeline_=nullptr;
	ShaderResourceBinding *fwd_srb_=nullptr, *bwd_srb_=nullptr;
};

class SoftmaxCrossEntropyOutputLayer : public ILayer {
public:
	SoftmaxCrossEntropyOutputLayer(uint32_t in_dim, uint32_t num_classes, uint32_t max_batch_size);
	~SoftmaxCrossEntropyOutputLayer() override;
	void Forward(CommandList*, Tensor& hidden_act) override;
	void Backward(CommandList*, const Tensor&, const Tensor& input_act) override;
	void ZeroGradients(CommandList*) override;
	void UpdateWeights(CommandList*, IOptimizer&, float inv_bs, uint32_t step) override;
	Tensor& GetOutput() override { return dL_dz_; }
	Tensor& GetInputGradient() override { return dL_dhidden_; }
	float GetLoss() const;
	void ZeroLossBuffer() { loss_buf_.Zero(); }
		Buffer* GetLossBuffer() const { return loss_buf_.GetBuffer(); }
	uint32_t NumClasses() const { return num_classes_; }
	ShaderResourceBinding* GetFwdLossSRB() { return fwd_loss_srb_; }
	std::vector<std::tuple<Tensor*, Tensor*, Tensor*>> GetParamTriples() override;
private:
	uint32_t in_dim_, num_classes_, max_batch_size_;
	Tensor weight_, bias_, grad_w_, grad_b_, v_w_, v_b_;
	Tensor dL_dz_, loss_buf_, dL_dhidden_;
	Tensor pc_buf_; // params for binding 10
	RenderPipelineState *fwd_loss_pipeline_=nullptr, *bwd_pipeline_=nullptr;
	ShaderResourceBinding *fwd_loss_srb_=nullptr, *bwd_srb_=nullptr;
};

} // namespace MXNN
