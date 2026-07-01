#include "AttentionLayer.h"
#include "ShaderHelper.h"
#include "Initializer.h"
#include "Optimizer.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {

// Attention parameter buffer for binding 10 (fwd: binding 10, bwd: binding 10)
// pc.d[0]=B, pc.d[1]=T, pc.d[2]=d, pc.d[3]=num_heads, pc.d[4]=d_k
struct AttnParams { Float32 B, T, d, num_heads, d_k; };

// ============================================================
// MultiHeadAttentionLayer
// ============================================================

MultiHeadAttentionLayer::MultiHeadAttentionLayer(
	UInt32 in_d_model, UInt32 in_num_heads,
	UInt32 in_max_batch, UInt32 in_max_seq_len)
	: d_model_(in_d_model)
	, num_heads_(in_num_heads)
	, head_dim_(in_d_model / in_num_heads)
	, max_batch_(in_max_batch)
	, max_seq_len_(in_max_seq_len)
	// Parameters
	, W_qkv_(Vector<UInt32>{d_model_, 3u * d_model_})
	, b_qkv_(Vector<UInt32>{3u * d_model_})
	, W_o_(Vector<UInt32>{d_model_, d_model_})
	, b_o_(Vector<UInt32>{d_model_})
	// Gradient buffers (use int for atomic float-add compatibility)
	, grad_W_qkv_(Vector<UInt32>{d_model_, 3u * d_model_})
	, grad_b_qkv_(Vector<UInt32>{3u * d_model_})
	, grad_W_o_(Vector<UInt32>{d_model_, d_model_})
	, grad_b_o_(Vector<UInt32>{d_model_})
	// SGD momentum
	, v_W_qkv_(Vector<UInt32>{d_model_, 3u * d_model_})
	, v_b_qkv_(Vector<UInt32>{3u * d_model_})
	, v_W_o_(Vector<UInt32>{d_model_, d_model_})
	, v_b_o_(Vector<UInt32>{d_model_})
	// Adam second moments
	// Forward activations
	, output_(Vector<UInt32>{max_batch_, max_seq_len_, d_model_})
	, dL_dx_(Vector<UInt32>{max_batch_, max_seq_len_, d_model_})
	, attn_weights_(Vector<UInt32>{max_batch_ * num_heads_, max_seq_len_, max_seq_len_})
	// Push-constant buffer
	, pc_buf_(Vector<UInt32>{5})
{
	// Initialize weights with Xavier uniform
	Float32 xavier_qkv = std::sqrt(6.0f / static_cast<Float32>(d_model_ + 3 * d_model_));
	Float32 xavier_o   = std::sqrt(6.0f / static_cast<Float32>(d_model_ + d_model_));

	W_qkv_.RandomNormal(0.0f, xavier_qkv);
	W_o_.RandomNormal(0.0f, xavier_o);
	InitializeBias(b_qkv_, 0.0f);
	InitializeBias(b_o_, 0.0f);

	// Zero out gradients
	{
		Vector<Float32> z_wqkv(grad_W_qkv_.ElementCount(), 0.0f);
		grad_W_qkv_.Upload(z_wqkv.data());
	}
	{
		Vector<Float32> z_bqkv(grad_b_qkv_.ElementCount(), 0.0f);
		grad_b_qkv_.Upload(z_bqkv.data());
	}
	{
		Vector<Float32> z_wo(grad_W_o_.ElementCount(), 0.0f);
		grad_W_o_.Upload(z_wo.data());
	}
	{
		Vector<Float32> z_bo(grad_b_o_.ElementCount(), 0.0f);
		grad_b_o_.Upload(z_bo.data());
	}

	// Zero out momentum
	{
		Vector<Float32> z(v_W_qkv_.ElementCount(), 0.0f); v_W_qkv_.Upload(z.data()); v_b_qkv_.Upload(z.data());
	}
	{
		Vector<Float32> z2(v_W_o_.ElementCount(), 0.0f);
		v_W_o_.Upload(z2.data()); v_b_o_.Upload(z2.data());
	}

	// Upload initial PC data
	AttnParams pc;
	pc.B = static_cast<Float32>(max_batch_);
	pc.T = static_cast<Float32>(max_seq_len_);
	pc.d = static_cast<Float32>(d_model_);
	pc.num_heads = static_cast<Float32>(num_heads_);
	pc.d_k = static_cast<Float32>(head_dim_);
	pc_buf_.Upload(&pc.B);

	CreatePipelinesAndSRBs();
}

MultiHeadAttentionLayer::~MultiHeadAttentionLayer()
{
	for (auto* s : temp_srbs_) delete s;
	if (bwd_srb_) delete bwd_srb_;
	if (fwd_srb_) delete fwd_srb_;
	// Pipelines are managed by VK_PipelineStateManager cache — do NOT delete
}

void MultiHeadAttentionLayer::CreatePipelinesAndSRBs()
{
	// Forward pipeline & SRB
	Shader* fwd_cs = LoadComputeShader("Shader/nn_attention_fwd.comp.spv");
	fwd_pipeline_ = CreateComputePipeline(fwd_cs);
	fwd_pipeline_->CreateShaderResourceBinding(fwd_srb_, false);

	// Bind static resources
	fwd_srb_->SetResource("W_qkv", W_qkv_.GetBuffer());
	fwd_srb_->SetResource("b_qkv", b_qkv_.GetBuffer());
	fwd_srb_->SetResource("W_o", W_o_.GetBuffer());
	fwd_srb_->SetResource("b_o", b_o_.GetBuffer());
	fwd_srb_->SetResource("out_buf", output_.GetBuffer());
	fwd_srb_->SetResource("attn_weights", attn_weights_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());

	// Backward pipeline & SRB
	Shader* bwd_cs = LoadComputeShader("Shader/nn_attention_bwd.comp.spv");
	bwd_pipeline_ = CreateComputePipeline(bwd_cs);
	bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_, false);

	// Bind static resources for backward
	bwd_srb_->SetResource("W_qkv", W_qkv_.GetBuffer());
	bwd_srb_->SetResource("W_o", W_o_.GetBuffer());
	bwd_srb_->SetResource("b_qkv", b_qkv_.GetBuffer());
	bwd_srb_->SetResource("attn_weights", attn_weights_.GetBuffer());
	bwd_srb_->SetResource("grad_W_qkv", grad_W_qkv_.GetBuffer());
	bwd_srb_->SetResource("grad_b_qkv", grad_b_qkv_.GetBuffer());
	bwd_srb_->SetResource("grad_W_o", grad_W_o_.GetBuffer());
	bwd_srb_->SetResource("grad_b_o", grad_b_o_.GetBuffer());
	bwd_srb_->SetResource("dL_dx", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());

	delete fwd_cs;
	delete bwd_cs;
}

void MultiHeadAttentionLayer::Forward(CommandList* in_cmd, Tensor& in_input)
{
	// B = total_tokens / T, works for both [B,T,d] and [B*T,d] input shapes
	UInt32 active_batch = static_cast<UInt32>(in_input.ElementCount()) / d_model_ / max_seq_len_;

	// Update per-call bindings + pc, then flush BEFORE bind (NV driver rule)
	AttnParams pc;
	pc.B = static_cast<Float32>(active_batch);
	pc.T = static_cast<Float32>(max_seq_len_);
	pc.d = static_cast<Float32>(d_model_);
	pc.num_heads = static_cast<Float32>(num_heads_);
	pc.d_k = static_cast<Float32>(head_dim_);
	pc_buf_.Upload(&pc.B);

	fwd_srb_->SetResource("inp", in_input.GetBuffer());
	fwd_srb_->FlushDescriptorWrites();

	in_cmd->SetComputePipeline(fwd_pipeline_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch(active_batch * num_heads_, 1u, 1u);
}

void MultiHeadAttentionLayer::Backward(CommandList* in_cmd,
	CONST Tensor& in_dL_dout, CONST Tensor& in_input_act)
{
	// B = total_tokens / T, works for both [B,T,d] and [B*T,d] input shapes
	UInt32 active_batch = static_cast<UInt32>(in_input_act.ElementCount()) / d_model_ / max_seq_len_;

	AttnParams pc;
	pc.B = static_cast<Float32>(active_batch);
	pc.T = static_cast<Float32>(max_seq_len_);
	pc.d = static_cast<Float32>(d_model_);
	pc.num_heads = static_cast<Float32>(num_heads_);
	pc.d_k = static_cast<Float32>(head_dim_);
	pc_buf_.Upload(&pc.B);

	// Zero gradient buffers before atomic accumulation in shader
	Vector<Float32> z(grad_W_qkv_.ElementCount(), 0.0f);
	grad_W_qkv_.Upload(z.data()); grad_b_qkv_.Upload(z.data());
	grad_W_o_.Upload(z.data()); grad_b_o_.Upload(z.data());

	bwd_srb_->SetResource("inp", in_input_act.GetBuffer());
	bwd_srb_->SetResource("dL_dout", in_dL_dout.GetBuffer());
	bwd_srb_->FlushDescriptorWrites();

	in_cmd->SetComputePipeline(bwd_pipeline_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch(active_batch * num_heads_, 1u, 1u);
}

void MultiHeadAttentionLayer::ZeroGradients(CommandList* /*in_cmd*/)
{
	// Handled by SequentialModel::ZeroAllGradients via GetParamTriples
}

void MultiHeadAttentionLayer::UpdateWeights(CommandList* in_cmd,
	IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step)
{
	in_opt.Update(in_cmd, W_qkv_, grad_W_qkv_, v_W_qkv_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, b_qkv_, grad_b_qkv_, v_b_qkv_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, W_o_, grad_W_o_, v_W_o_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, b_o_, grad_b_o_, v_b_o_, in_inv_bs, in_step);
}

Vector<std::tuple<Tensor*, Tensor*, Tensor*>> MultiHeadAttentionLayer::GetParamTriples()
{
	return {
		{&W_qkv_, &grad_W_qkv_, &v_W_qkv_},
		{&b_qkv_, &grad_b_qkv_, &v_b_qkv_},
		{&W_o_,   &grad_W_o_,   &v_W_o_},
		{&b_o_,   &grad_b_o_,   &v_b_o_}
	};
}

void MultiHeadAttentionLayer::SaveParameters(std::ostream& os) const {
    os.write((char*)&d_model_, 4); os.write((char*)&num_heads_, 4);
    os.write((char*)&max_batch_, 4); os.write((char*)&max_seq_len_, 4);
    WriteTensor(os, W_qkv_); WriteTensor(os, b_qkv_);
    WriteTensor(os, W_o_); WriteTensor(os, b_o_);
}
void MultiHeadAttentionLayer::LoadParameters(std::istream& is) {
    ReadTensor(is, W_qkv_); ReadTensor(is, b_qkv_);
    ReadTensor(is, W_o_); ReadTensor(is, b_o_);
}

} // namespace MXNN
