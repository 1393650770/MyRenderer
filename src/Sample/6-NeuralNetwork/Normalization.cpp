#include "Normalization.h"
#include "ShaderHelper.h"
#include "Initializer.h"
#include "Optimizer.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"
// -- [AI:BEGIN]

namespace MXNN {

// ======== DropoutLayer ========
DropoutLayer::DropoutLayer(Float32 in_p, UInt32 in_n_elem)
	: p_(in_p), keep_prob_(1.0f - in_p), n_elem_(in_n_elem), seed_(0u), training_(true)
	, output_(Vector<UInt32>{in_n_elem}), dL_dx_(Vector<UInt32>{in_n_elem}), mask_(Vector<UInt32>{in_n_elem})
{
	Shader* s = LoadComputeShader("Shader/nn_dropout.comp.spv");
	pipeline_ = CreateComputePipeline(s);
	pipeline_->CreateShaderResourceBinding(fwd_srb_, false);
	pipeline_->CreateShaderResourceBinding(bwd_srb_, false); // -- [AI]
	delete s;
}

DropoutLayer::~DropoutLayer() {
	for (auto* s : temp_srbs_) delete s;
	if (bwd_srb_) delete bwd_srb_;
	if (fwd_srb_) delete fwd_srb_;
	// pipeline_ is managed by VK_PipelineStateManager cache — do NOT delete
}

void DropoutLayer::SetTrainingMode(Bool in_training) { training_ = in_training; }
void DropoutLayer::SetSeed(UInt32 in_seed) { seed_ = in_seed; }

void DropoutLayer::Forward(CommandList* in_cmd, Tensor& in_input) {
	Float32 pc[4] = { static_cast<Float32>(n_elem_), training_ ? 0.0f : 2.0f, keep_prob_, static_cast<Float32>(seed_) };
	pc_buf_.Upload(pc);
	// -- [AI:BEGIN] reuse static srb_
	fwd_srb_->SetResource("inp", in_input.GetBuffer());
	fwd_srb_->SetResource("out_buf", output_.GetBuffer());
	fwd_srb_->SetResource("mask", mask_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	fwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(pipeline_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
	// -- [AI:END]
}

void DropoutLayer::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& /*input_act*/) {
	Float32 pc[4] = { static_cast<Float32>(n_elem_), 1.0f, keep_prob_, static_cast<Float32>(seed_) };
	pc_buf_.Upload(pc);
	// -- [AI:BEGIN] reuse static srb_
	bwd_srb_->SetResource("inp", in_dL_dout.GetBuffer());
	bwd_srb_->SetResource("out_buf", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("mask", mask_.GetBuffer());
	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	bwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(pipeline_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
	// -- [AI:END]
}

// -- [AI] DropoutLayer Persistence
void DropoutLayer::SaveParameters(std::ostream& os) const {
    os.write((char*)&p_, 4); os.write((char*)&n_elem_, 4);
}
void DropoutLayer::LoadParameters(std::istream& is) {
    // params p_ and n_elem_ are already set by constructor in factory
}

// ======== LayerNorm ========
LayerNorm::LayerNorm(UInt32 in_n_features, UInt32 in_max_batch, Float32 in_eps)
	: n_features_(in_n_features), max_batch_(in_max_batch), n_elem_(in_max_batch * in_n_features)
	, eps_(in_eps)
	, gamma_(Vector<UInt32>{in_n_features}), beta_(Vector<UInt32>{in_n_features})
	, grad_gamma_(Vector<UInt32>{in_n_features}), grad_beta_(Vector<UInt32>{in_n_features})
	, v_gamma_(Vector<UInt32>{in_n_features}), v_beta_(Vector<UInt32>{in_n_features})
	, output_(Vector<UInt32>{in_max_batch, in_n_features}), dL_dx_(Vector<UInt32>{in_max_batch, in_n_features})
	, saved_mu_(Vector<UInt32>{in_max_batch}), saved_std_(Vector<UInt32>{in_max_batch})
{
	InitializeBias(beta_, 0.0f);
	{ Vector<Float32> ones(in_n_features, 1.0f); gamma_.Upload(ones.data()); }
	Float32 pc[2] = { static_cast<Float32>(n_features_), eps_ };
	pc_buf_.Upload(pc);
	Shader* fs = LoadComputeShader("Shader/nn_layernorm_fwd.comp.spv");
	fwd_pipe_ = CreateComputePipeline(fs); fwd_pipe_->CreateShaderResourceBinding(fwd_srb_, false); delete fs;
	Shader* bs = LoadComputeShader("Shader/nn_layernorm_bwd.comp.spv");
	bwd_pipe_ = CreateComputePipeline(bs); bwd_pipe_->CreateShaderResourceBinding(bwd_srb_, false); delete bs;
}

LayerNorm::~LayerNorm() {
	for (auto* s : temp_srbs_) delete s;
	delete bwd_srb_; delete fwd_srb_;
}

void LayerNorm::Forward(CommandList* in_cmd, Tensor& in_input) {
	// -- [AI:BEGIN] reuse static fwd_srb_
	fwd_srb_->SetResource("inp", in_input.GetBuffer());
	fwd_srb_->SetResource("out_buf", output_.GetBuffer());
	fwd_srb_->SetResource("gamma", gamma_.GetBuffer());
	fwd_srb_->SetResource("beta", beta_.GetBuffer());
	fwd_srb_->SetResource("saved_mu", saved_mu_.GetBuffer());
	fwd_srb_->SetResource("saved_std", saved_std_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	fwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(fwd_pipe_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch(max_batch_, 1u, 1u);
	// -- [AI:END]
}

void LayerNorm::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) {
	// -- [AI:BEGIN] reuse static bwd_srb_
	bwd_srb_->SetResource("dL_dout", in_dL_dout.GetBuffer());
	bwd_srb_->SetResource("dL_dx", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("inp", in_input_act.GetBuffer());
	bwd_srb_->SetResource("gamma", gamma_.GetBuffer());
	bwd_srb_->SetResource("saved_mu", saved_mu_.GetBuffer());
	bwd_srb_->SetResource("saved_std", saved_std_.GetBuffer());
	bwd_srb_->SetResource("grad_gamma", grad_gamma_.GetBuffer());
	bwd_srb_->SetResource("grad_beta", grad_beta_.GetBuffer());
	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	bwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(bwd_pipe_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch(max_batch_, 1u, 1u);
	// -- [AI:END]
}

void LayerNorm::ZeroGradients(CommandList* in_cmd) {
	// Handled by SequentialModel::ZeroAllGradients via GetParamTriples
}

void LayerNorm::UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) {
	in_opt.Update(in_cmd, gamma_, grad_gamma_, v_gamma_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, beta_, grad_beta_, v_beta_, in_inv_bs, in_step);
}

Vector<std::tuple<Tensor*,Tensor*,Tensor*>> LayerNorm::GetParamTriples() {
	return {{&gamma_, &grad_gamma_, &v_gamma_}, {&beta_, &grad_beta_, &v_beta_}};
}

// -- [AI] LayerNorm Persistence
void LayerNorm::SaveParameters(std::ostream& os) const {
    os.write((char*)&n_features_, 4); os.write((char*)&max_batch_, 4); os.write((char*)&eps_, 4);
    WriteTensor(os, gamma_); WriteTensor(os, beta_);
}
void LayerNorm::LoadParameters(std::istream& is) {
    ReadTensor(is, gamma_); ReadTensor(is, beta_);
}

// ======== BatchNorm1DLayer ========
BatchNorm1DLayer::BatchNorm1DLayer(UInt32 in_n_features, UInt32 in_max_batch, Float32 in_momentum, Float32 in_eps)
	: n_features_(in_n_features), max_batch_(in_max_batch), n_elem_(in_max_batch * in_n_features)
	, momentum_(in_momentum), eps_(in_eps), training_(true)
	, gamma_(Vector<UInt32>{in_n_features}), beta_(Vector<UInt32>{in_n_features})
	, grad_gamma_(Vector<UInt32>{in_n_features}), grad_beta_(Vector<UInt32>{in_n_features})
	, v_gamma_(Vector<UInt32>{in_n_features}), v_beta_(Vector<UInt32>{in_n_features})
	, running_mean_(Vector<UInt32>{in_n_features}), running_var_(Vector<UInt32>{in_n_features})
	, output_(Vector<UInt32>{in_max_batch, in_n_features}), dL_dx_(Vector<UInt32>{in_max_batch, in_n_features})
	, saved_mu_(Vector<UInt32>{in_n_features}), saved_std_(Vector<UInt32>{in_n_features})
{
	{ Vector<Float32> zero(in_n_features, 0.0f); running_mean_.Upload(zero.data()); }
	{ Vector<Float32> ones(in_n_features, 1.0f); running_var_.Upload(ones.data()); gamma_.Upload(ones.data()); }
	InitializeBias(beta_, 0.0f);
	Float32 pc[5] = { static_cast<Float32>(max_batch_), static_cast<Float32>(n_features_), eps_, momentum_, 1.0f };
	pc_buf_.Upload(pc);
	Shader* fs = LoadComputeShader("Shader/nn_batchnorm_fwd.comp.spv");
	fwd_pipe_ = CreateComputePipeline(fs); fwd_pipe_->CreateShaderResourceBinding(fwd_srb_, false); delete fs;
	Shader* bs = LoadComputeShader("Shader/nn_batchnorm_bwd.comp.spv");
	bwd_pipe_ = CreateComputePipeline(bs); bwd_pipe_->CreateShaderResourceBinding(bwd_srb_, false); delete bs;
}

BatchNorm1DLayer::~BatchNorm1DLayer() {
	for (auto* s : temp_srbs_) delete s;
	delete bwd_srb_; delete fwd_srb_;
}

void BatchNorm1DLayer::SetTrainingMode(Bool in_training) {
	training_ = in_training;
	Float32 pc[5] = { static_cast<Float32>(max_batch_), static_cast<Float32>(n_features_), eps_, momentum_, training_ ? 1.0f : 0.0f };
	pc_buf_.Upload(pc);
}

void BatchNorm1DLayer::Forward(CommandList* in_cmd, Tensor& in_input) {
	// -- [AI:BEGIN] reuse static fwd_srb_
	fwd_srb_->SetResource("inp", in_input.GetBuffer());
	fwd_srb_->SetResource("out_buf", output_.GetBuffer());
	fwd_srb_->SetResource("gamma", gamma_.GetBuffer());
	fwd_srb_->SetResource("beta", beta_.GetBuffer());
	fwd_srb_->SetResource("running_mean", running_mean_.GetBuffer());
	fwd_srb_->SetResource("running_var", running_var_.GetBuffer());
	fwd_srb_->SetResource("saved_mu", saved_mu_.GetBuffer());
	fwd_srb_->SetResource("saved_std", saved_std_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	fwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(fwd_pipe_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch(n_features_, 1u, 1u);
	// -- [AI:END]
}

void BatchNorm1DLayer::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) {
	// -- [AI:BEGIN] reuse static bwd_srb_
	bwd_srb_->SetResource("dL_dout", in_dL_dout.GetBuffer());
	bwd_srb_->SetResource("dL_dx", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("inp", in_input_act.GetBuffer());
	bwd_srb_->SetResource("gamma", gamma_.GetBuffer());
	bwd_srb_->SetResource("saved_mu", saved_mu_.GetBuffer());
	bwd_srb_->SetResource("saved_std", saved_std_.GetBuffer());
	bwd_srb_->SetResource("grad_gamma", grad_gamma_.GetBuffer());
	bwd_srb_->SetResource("grad_beta", grad_beta_.GetBuffer());
	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	bwd_srb_->FlushDescriptorWrites();
	in_cmd->SetComputePipeline(bwd_pipe_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch(n_features_, 1u, 1u);
	// -- [AI:END]
}

void BatchNorm1DLayer::ZeroGradients(CommandList*) {}
void BatchNorm1DLayer::UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) {
	in_opt.Update(in_cmd, gamma_, grad_gamma_, v_gamma_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, beta_, grad_beta_, v_beta_, in_inv_bs, in_step);
}

Vector<std::tuple<Tensor*,Tensor*,Tensor*>> BatchNorm1DLayer::GetParamTriples() {
	return {{&gamma_, &grad_gamma_, &v_gamma_}, {&beta_, &grad_beta_, &v_beta_}};
}

// -- [AI] BatchNorm1DLayer Persistence
void BatchNorm1DLayer::SaveParameters(std::ostream& os) const {
    os.write((char*)&n_features_, 4); os.write((char*)&max_batch_, 4);
    os.write((char*)&momentum_, 4); os.write((char*)&eps_, 4);
    WriteTensor(os, gamma_); WriteTensor(os, beta_);
    WriteTensor(os, running_mean_); WriteTensor(os, running_var_);
}
void BatchNorm1DLayer::LoadParameters(std::istream& is) {
    ReadTensor(is, gamma_); ReadTensor(is, beta_);
    ReadTensor(is, running_mean_); ReadTensor(is, running_var_);
}

} // namespace MXNN