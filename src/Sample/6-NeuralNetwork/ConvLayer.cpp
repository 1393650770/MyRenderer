#include "ConvLayer.h"

#include "ShaderHelper.h"

#include "Initializer.h"

#include "Optimizer.h"

#include "RHI/RenderCommandList.h"

#include "RHI/RenderPipelineState.h"

#include <cmath>

// --  



namespace MXNN {



// ============================================================

// Conv2DLayer

// ============================================================



Conv2DLayer::Conv2DLayer(UInt32 in_in_channels, UInt32 in_out_channels,

                         UInt32 in_kernel_h, UInt32 in_kernel_w,

                         UInt32 in_input_h, UInt32 in_input_w,

                         UInt32 in_max_batch,

                         UInt32 in_stride, UInt32 in_padding)

	: in_channels_(in_in_channels)

	, out_channels_(in_out_channels)

	, kernel_h_(in_kernel_h)

	, kernel_w_(in_kernel_w)

	, stride_(in_stride)

	, padding_(in_padding)

	, input_h_(in_input_h)

	, input_w_(in_input_w)

	, output_h_(ComputeOutputSize(in_input_h, in_kernel_h, in_stride, in_padding))

	, output_w_(ComputeOutputSize(in_input_w, in_kernel_w, in_stride, in_padding))

	, max_batch_(in_max_batch)

	, weight_(Vector<UInt32>{out_channels_, kernel_h_, kernel_w_, in_channels_})

	, bias_(Vector<UInt32>{out_channels_})

	, grad_w_(Vector<UInt32>{out_channels_, kernel_h_, kernel_w_, in_channels_})

	, grad_b_(Vector<UInt32>{out_channels_})

	, v_w_(Vector<UInt32>{out_channels_, kernel_h_, kernel_w_, in_channels_})

	, v_b_(Vector<UInt32>{out_channels_})

	, output_(Vector<UInt32>{max_batch_, output_h_, output_w_, out_channels_})

	, dL_dx_(Vector<UInt32>{max_batch_, input_h_, input_w_, in_channels_})

{

	// Xavier init for weight

	Float32 k = 1.0f / static_cast<Float32>(kernel_h_ * kernel_w_ * in_channels_);

	weight_.RandomNormal(0.0f, std::sqrt(k));

	// --   Use existing Zero() instead of InitializeBias which may not be available

	bias_.Zero();

	CreatePipelineAndSRB();

}



Conv2DLayer::~Conv2DLayer()

{

	// --   Static SRBs: no temp_srbs_ to clean up

	if (bwd_srb_) delete bwd_srb_;

	if (fwd_srb_) delete fwd_srb_;

	// Pipelines are managed by VK_PipelineStateManager cache -- do NOT delete

}



UInt32 Conv2DLayer::ComputeOutputSize(UInt32 in_size, UInt32 in_kernel, UInt32 in_stride, UInt32 in_padding) const

{

	return (in_size + 2u * in_padding - in_kernel) / in_stride + 1u;

}



void Conv2DLayer::CreatePipelineAndSRB()

{

	// Forward pipeline & SRB --   static bindings for weight, bias, output, pc (same pattern as LinearLayer)

	Shader* fwd_shader = LoadComputeShader("Shader/nn_conv2d_fwd.comp.spv");

	fwd_pipeline_ = CreateComputePipeline(fwd_shader);

	fwd_pipeline_->CreateShaderResourceBinding(fwd_srb_, false);


	fwd_srb_->SetResource("weight", weight_.GetBuffer());

	fwd_srb_->SetResource("bias", bias_.GetBuffer());

	fwd_srb_->SetResource("out_buf", output_.GetBuffer());

	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());


	// Backward pipeline & SRB --   static bindings for dL_dx, weight, grad_w, grad_b, pc (same pattern as LinearLayer)

	Shader* bwd_shader = LoadComputeShader("Shader/nn_conv2d_bwd.comp.spv");

	bwd_pipeline_ = CreateComputePipeline(bwd_shader);

	bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_, false);


	bwd_srb_->SetResource("dL_dx", dL_dx_.GetBuffer());

	bwd_srb_->SetResource("weight", weight_.GetBuffer());

	bwd_srb_->SetResource("grad_w", grad_w_.GetBuffer());

	bwd_srb_->SetResource("grad_b", grad_b_.GetBuffer());

	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());


	delete fwd_shader;

	delete bwd_shader;

}



void Conv2DLayer::Forward(CommandList* in_cmd, Tensor& in_input)

{

	Float32 pc[10];

	pc[0] = static_cast<Float32>(max_batch_);

	pc[1] = static_cast<Float32>(in_channels_);

	pc[2] = static_cast<Float32>(out_channels_);

	pc[3] = static_cast<Float32>(input_h_);

	pc[4] = static_cast<Float32>(input_w_);

	pc[5] = static_cast<Float32>(kernel_h_);

	pc[6] = static_cast<Float32>(kernel_w_);

	pc[7] = static_cast<Float32>(stride_);

	pc[8] = static_cast<Float32>(padding_);

	pc[9] = static_cast<Float32>(output_h_ * output_w_);

	pc_buf_.Upload(pc);


	// --   Use static fwd_srb_ -- only set per-call input, same pattern as LinearLayer

	fwd_srb_->SetResource("inp", in_input.GetBuffer());


	in_cmd->SetComputePipeline(fwd_pipeline_);

	in_cmd->SetShaderResourceBinding(fwd_srb_);

	UInt32 gx = (out_channels_ + 3u) / 4u;

	UInt32 gy = (output_h_ + 3u) / 4u;

	UInt32 gz = (max_batch_ * output_w_ + 3u) / 4u;

	in_cmd->Dispatch(gx, gy, gz);

}



void Conv2DLayer::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act)

{

	Float32 pc[10];

	pc[0] = static_cast<Float32>(max_batch_);

	pc[1] = static_cast<Float32>(in_channels_);

	pc[2] = static_cast<Float32>(out_channels_);

	pc[3] = static_cast<Float32>(input_h_);

	pc[4] = static_cast<Float32>(input_w_);

	pc[5] = static_cast<Float32>(kernel_h_);

	pc[6] = static_cast<Float32>(kernel_w_);

	pc[7] = static_cast<Float32>(stride_);

	pc[8] = static_cast<Float32>(padding_);

	pc[9] = static_cast<Float32>(output_h_ * output_w_);

	pc_buf_.Upload(pc);


	// Zero grad buffers before atomic accumulation

	grad_b_.Zero();

	grad_w_.Zero();

	dL_dx_.Zero();


	// --   Use static bwd_srb_ -- only set per-call inputs, same pattern as LinearLayer

	bwd_srb_->SetResource("dL_dout", in_dL_dout.GetBuffer());

	bwd_srb_->SetResource("inp", in_input_act.GetBuffer());


	in_cmd->SetComputePipeline(bwd_pipeline_);

	in_cmd->SetShaderResourceBinding(bwd_srb_);

	UInt32 gx = (out_channels_ + 3u) / 4u;

	UInt32 gy = (output_h_ + 3u) / 4u;

	UInt32 gz = (max_batch_ * output_w_ + 3u) / 4u;

	in_cmd->Dispatch(gx, gy, gz);

}



void Conv2DLayer::ZeroGradients(CommandList* /*in_cmd*/)

{

	// Handled by SequentialModel::ZeroAllGradients via zero_grad shader

}



void Conv2DLayer::UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step)

{

	in_opt.Update(in_cmd, weight_, grad_w_, v_w_, in_inv_bs, in_step);

	in_opt.Update(in_cmd, bias_, grad_b_, v_b_, in_inv_bs, in_step);

}



Vector<std::tuple<Tensor*, Tensor*, Tensor*>> Conv2DLayer::GetParamTriples()

{

	return {

		{&weight_, &grad_w_, &v_w_},

		{&bias_,   &grad_b_, &v_b_}

	};

}



// --   Persistence

void Conv2DLayer::SaveParameters(std::ostream& os) const {

	os.write((char*)&in_channels_, 4); os.write((char*)&out_channels_, 4);

	os.write((char*)&kernel_h_, 4); os.write((char*)&kernel_w_, 4);

	os.write((char*)&input_h_, 4); os.write((char*)&input_w_, 4);

	os.write((char*)&max_batch_, 4); os.write((char*)&stride_, 4); os.write((char*)&padding_, 4);

	WriteTensor(os, weight_); WriteTensor(os, bias_);

}

void Conv2DLayer::LoadParameters(std::istream& is) {

	ReadTensor(is, weight_); ReadTensor(is, bias_);

}

// --  


} // namespace MXNN


// --  

