#pragma once
// -- [AI:BEGIN]
#ifndef _NN_ATTENTIONLAYER_
#define _NN_ATTENTIONLAYER_
#include "Layer.h"

namespace MXNN {

// ============================================================
// MultiHeadAttentionLayer
//
// Single-head attention per dispatch, combined QKV projection.
// Input shape: [B, T, d_model]
// W_qkv: [d_model, 3*d_model]  combined Q,K,V weights
// W_o:   [d_model, d_model]    output projection
// d_k = d_model / num_heads    (head dimension)
//
// Forward calls: B * num_heads dispatches of size (1,1,1)
// Backward calls: same dispatch pattern
// ============================================================
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(MultiHeadAttentionLayer, public MXNN::ILayer)
#pragma region MATHOD
public:
	MultiHeadAttentionLayer(UInt32 in_d_model, UInt32 in_num_heads,
		UInt32 in_max_batch, UInt32 in_max_seq_len);
	VIRTUAL ~MultiHeadAttentionLayer();

	VIRTUAL void METHOD(Forward)(CommandList* in_cmd, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void METHOD(Backward)(CommandList* in_cmd, CONST Tensor& in_dL_dout,
		CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void METHOD(ZeroGradients)(CommandList* in_cmd) OVERRIDE FINAL;
	VIRTUAL void METHOD(UpdateWeights)(CommandList* in_cmd, IOptimizer& in_opt,
		Float32 in_inv_bs, UInt32 in_step) OVERRIDE FINAL;

	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL;

	// -- [AI] Persistence
	VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "MultiHeadAttention"; }
	VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
	VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;

	UInt32 DModel() CONST { return d_model_; }
	UInt32 NumHeads() CONST { return num_heads_; }
	UInt32 HeadDim() CONST { return head_dim_; }
protected:
private:
	void CreatePipelinesAndSRBs();
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	UInt32 d_model_;
	UInt32 num_heads_;
	UInt32 head_dim_;     // d_k = d_model / num_heads
	UInt32 max_batch_;
	UInt32 max_seq_len_;

	// -- [AI] Parameters
	Tensor W_qkv_;       // [d_model, 3*d_model]
	Tensor b_qkv_;       // [3*d_model]
	Tensor W_o_;         // [d_model, d_model]
	Tensor b_o_;         // [d_model]

	// -- [AI] Gradients (stored as int for atomic compat; zeroed/converted as needed)
	Tensor grad_W_qkv_;  // [d_model, 3*d_model]
	Tensor grad_b_qkv_;  // [3*d_model]
	Tensor grad_W_o_;    // [d_model, d_model]
	Tensor grad_b_o_;    // [d_model]

	// -- [AI] SGD momentum
	Tensor v_W_qkv_;     // [d_model, 3*d_model]
	Tensor v_b_qkv_;     // [3*d_model]
	Tensor v_W_o_;       // [d_model, d_model]
	Tensor v_b_o_;       // [d_model]


	// -- [AI] Forward/backward activations and gradients
	Tensor output_;      // [B, T, d_model]
	Tensor dL_dx_;       // [B, T, d_model]
	Tensor attn_weights_; // [B * num_heads, T, T]  saved softmax output

	// -- [AI] Push-constant buffer
	Tensor pc_buf_;      // 5 floats: B, T, d, num_heads, d_k

	// -- [AI] Pipelines and SRBs
	RenderPipelineState* fwd_pipeline_ = nullptr;
	RenderPipelineState* bwd_pipeline_ = nullptr;
	ShaderResourceBinding* fwd_srb_ = nullptr;
	ShaderResourceBinding* bwd_srb_ = nullptr;
	Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
// -- [AI:END]
#endif
