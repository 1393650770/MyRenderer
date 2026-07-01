#pragma once
// -- [AI:BEGIN]
#ifndef _NN_NORMALIZATION_
#define _NN_NORMALIZATION_
#include "Layer.h"

namespace MXNN {

// ---- Dropout ----
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(DropoutLayer, public ILayer)
#pragma region MATHOD
public:
	DropoutLayer(Float32 in_p, UInt32 in_n_elem);
	VIRTUAL ~DropoutLayer();
	VIRTUAL void METHOD(Forward)(CommandList*, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void METHOD(Backward)(CommandList*, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void METHOD(ZeroGradients)(CommandList*) OVERRIDE FINAL {}
	VIRTUAL void METHOD(UpdateWeights)(CommandList*, IOptimizer&, Float32, UInt32) OVERRIDE FINAL {}
	VIRTUAL Vector<std::tuple<Tensor*,Tensor*,Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL { return {}; }
	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
			VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "Dropout"; }
			VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
			VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;
			VIRTUAL void METHOD(SetTrainingMode)(Bool in_training);
		void SetSeed(UInt32 in_seed);
protected:
	Float32 p_, keep_prob_;
	UInt32 n_elem_, seed_;
	Bool training_;
	Tensor output_, dL_dx_, mask_;
	Tensor pc_buf_{{4}};
	RenderPipelineState* pipeline_ = nullptr;
	ShaderResourceBinding* fwd_srb_ = nullptr;
		ShaderResourceBinding* bwd_srb_ = nullptr; // -- [AI]
	Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

// ---- LayerNorm ----
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(LayerNorm, public ILayer)
#pragma region MATHOD
public:
	LayerNorm(UInt32 in_n_features, UInt32 in_max_batch, Float32 in_eps = 1e-5f);
	VIRTUAL ~LayerNorm();
	VIRTUAL void METHOD(Forward)(CommandList*, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void METHOD(Backward)(CommandList*, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void METHOD(ZeroGradients)(CommandList*) OVERRIDE FINAL;
	VIRTUAL void METHOD(UpdateWeights)(CommandList*, IOptimizer&, Float32, UInt32) OVERRIDE FINAL;
	VIRTUAL Vector<std::tuple<Tensor*,Tensor*,Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL;
	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
	// -- [AI] Persistence
	VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "LayerNorm"; }
	VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
	VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;
protected:
	UInt32 n_features_, max_batch_, n_elem_;
	Float32 eps_;
	Tensor gamma_, beta_, grad_gamma_, grad_beta_, v_gamma_, v_beta_;
	Tensor output_, dL_dx_, saved_mu_, saved_std_;
	Tensor pc_buf_{{2}};
	RenderPipelineState *fwd_pipe_=nullptr, *bwd_pipe_=nullptr;
	ShaderResourceBinding *fwd_srb_=nullptr, *bwd_srb_=nullptr;
	Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

// ---- BatchNorm1D ----
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BatchNorm1DLayer, public ILayer)
#pragma region MATHOD
public:
	BatchNorm1DLayer(UInt32 in_n_features, UInt32 in_max_batch, Float32 in_momentum = 0.1f, Float32 in_eps = 1e-5f);
	VIRTUAL ~BatchNorm1DLayer();
	VIRTUAL void METHOD(Forward)(CommandList*, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void METHOD(Backward)(CommandList*, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void METHOD(ZeroGradients)(CommandList*) OVERRIDE FINAL;
	VIRTUAL void METHOD(UpdateWeights)(CommandList*, IOptimizer&, Float32, UInt32) OVERRIDE FINAL;
	VIRTUAL Vector<std::tuple<Tensor*,Tensor*,Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL;
	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
			VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "BatchNorm1D"; }
			VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
			VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;
			VIRTUAL void METHOD(SetTrainingMode)(Bool in_training);
protected:
	UInt32 n_features_, max_batch_, n_elem_;
	Float32 momentum_, eps_;
	Bool training_;
	Tensor gamma_, beta_, grad_gamma_, grad_beta_, v_gamma_, v_beta_;
	Tensor running_mean_, running_var_;
	Tensor output_, dL_dx_, saved_mu_, saved_std_;
	Tensor pc_buf_{{5}};
	RenderPipelineState *fwd_pipe_=nullptr, *bwd_pipe_=nullptr;
	ShaderResourceBinding *fwd_srb_=nullptr, *bwd_srb_=nullptr;
	Vector<ShaderResourceBinding*> temp_srbs_;
#pragma endregion
MYRENDERER_END_CLASS

} // namespace MXNN
// -- [AI:END]
#endif
