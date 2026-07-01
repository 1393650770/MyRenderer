#pragma once
// -- [AI:BEGIN]
#ifndef _NN_ACTIVATION_
#define _NN_ACTIVATION_
#include "Layer.h"

namespace MXNN {

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(ActivationLayer, public ILayer)
#pragma region MATHOD
public:
	ActivationLayer(CONST Vector<UInt32>& in_shape, CONST String& in_type_name, CONST String& in_fwd_shader, CONST String& in_bwd_shader);
	VIRTUAL ~ActivationLayer();
	VIRTUAL void METHOD(Forward)(CommandList* in_cmd, Tensor& in_input) OVERRIDE FINAL;
	VIRTUAL void METHOD(Backward)(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;
	VIRTUAL void METHOD(ZeroGradients)(CommandList*) OVERRIDE FINAL {}
	VIRTUAL void METHOD(UpdateWeights)(CommandList*, IOptimizer&, Float32, UInt32) OVERRIDE FINAL {}
	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL { return {}; }
	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }
	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }
protected:
	String type_name_;
	UInt32 n_elem_;
	Tensor output_, dL_dx_, saved_input_;
	Tensor pc_buf_{{1}};
	RenderPipelineState* fwd_pipeline_ = nullptr;
	RenderPipelineState* bwd_pipeline_ = nullptr;
	ShaderResourceBinding* fwd_srb_ = nullptr;
	ShaderResourceBinding* bwd_srb_ = nullptr;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(ReLULayer, public ActivationLayer)
public: ReLULayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "ReLU", "nn_act_relu_fwd.comp", "nn_act_relu_bwd.comp") {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(LeakyReLULayer, public ActivationLayer)
public: LeakyReLULayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "LeakyReLU", "nn_act_lrelu_fwd.comp", "nn_act_lrelu_bwd.comp") {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(SigmoidLayer, public ActivationLayer)
public: SigmoidLayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "Sigmoid", "nn_act_sigmoid_fwd.comp", "nn_act_sigmoid_bwd.comp") {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(TanhLayer, public ActivationLayer)
public: TanhLayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "Tanh", "nn_act_tanh_fwd.comp", "nn_act_tanh_bwd.comp") {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(GELULayer, public ActivationLayer)
public: GELULayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "GELU", "nn_act_gelu_fwd.comp", "nn_act_gelu_bwd.comp") {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(SiLULayer, public ActivationLayer)
public: SiLULayer(CONST Vector<UInt32>& in_shape) : ActivationLayer(in_shape, "SiLU", "nn_act_silu_fwd.comp", "nn_act_silu_bwd.comp") {}
MYRENDERER_END_CLASS

} // namespace MXNN
// -- [AI:END]
#endif
