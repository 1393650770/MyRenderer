#pragma once

// -- [AI:BEGIN]

#ifndef _NN_CONVLAYER_

#define _NN_CONVLAYER_

#include "Layer.h"



namespace MXNN {



MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Conv2DLayer, public ILayer)

#pragma region MATHOD

public:

	Conv2DLayer(UInt32 in_in_channels, UInt32 in_out_channels,

	            UInt32 in_kernel_h, UInt32 in_kernel_w,

	            UInt32 in_input_h, UInt32 in_input_w,

	            UInt32 in_max_batch,

	            UInt32 in_stride = 1u, UInt32 in_padding = 0u);

	VIRTUAL ~Conv2DLayer();



	VIRTUAL void METHOD(Forward)(CommandList* in_cmd, Tensor& in_input) OVERRIDE FINAL;

	VIRTUAL void METHOD(Backward)(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) OVERRIDE FINAL;

	VIRTUAL void METHOD(ZeroGradients)(CommandList* in_cmd) OVERRIDE FINAL;

	VIRTUAL void METHOD(UpdateWeights)(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) OVERRIDE FINAL;



	VIRTUAL Tensor& METHOD(GetOutput)() OVERRIDE FINAL { return output_; }

	VIRTUAL Tensor& METHOD(GetInputGradient)() OVERRIDE FINAL { return dL_dx_; }

	VIRTUAL Vector<std::tuple<Tensor*, Tensor*, Tensor*>> METHOD(GetParamTriples)() OVERRIDE FINAL;

		// -- [AI] Persistence
		VIRTUAL String METHOD(GetLayerTypeName)() CONST OVERRIDE FINAL { return "Conv2DLayer"; }
		VIRTUAL void METHOD(SaveParameters)(std::ostream& os) CONST OVERRIDE FINAL;
		VIRTUAL void METHOD(LoadParameters)(std::istream& is) OVERRIDE FINAL;

		UInt32 OutputHeight() const { return output_h_; }

	UInt32 OutputWidth() const { return output_w_; }



protected:

private:

	void CreatePipelineAndSRB();

	UInt32 ComputeOutputSize(UInt32 in_size, UInt32 in_kernel, UInt32 in_stride, UInt32 in_padding) const;

#pragma endregion



#pragma region MEMBER

public:

protected:

private:

	UInt32 in_channels_, out_channels_, kernel_h_, kernel_w_;

	UInt32 stride_, padding_;

	UInt32 input_h_, input_w_, output_h_, output_w_;

	UInt32 max_batch_;



	Tensor weight_, bias_, grad_w_, grad_b_, v_w_, v_b_;

	Tensor output_, dL_dx_;

	Tensor pc_buf_{{ {10} }};



	RenderPipelineState* fwd_pipeline_ = nullptr;

	RenderPipelineState* bwd_pipeline_ = nullptr;

	ShaderResourceBinding* fwd_srb_ = nullptr;

	ShaderResourceBinding* bwd_srb_ = nullptr;

	// -- [AI] temp_srbs_ removed -- static SRB pattern used instead

#pragma endregion

MYRENDERER_END_CLASS



} // namespace MXNN

// -- [AI:END]

#endif

