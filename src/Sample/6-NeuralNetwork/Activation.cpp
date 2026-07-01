#include "Activation.h"
#include "ShaderHelper.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"

namespace MXNN {

ActivationLayer::ActivationLayer(CONST Vector<UInt32>& in_shape, CONST String& in_type_name, CONST String& in_fwd_shader, CONST String& in_bwd_shader)
	: type_name_(in_type_name)
	, n_elem_(1)
	, output_(in_shape)
	, dL_dx_(in_shape)
	, saved_input_(in_shape)
{
	n_elem_ = 1; for (UInt32 d : in_shape) n_elem_ *= d;
	Float32 n_f = static_cast<Float32>(n_elem_);
	pc_buf_.Upload(&n_f);

	Shader* fs = LoadComputeShader("Shader/" + in_fwd_shader + ".spv");
	fwd_pipeline_ = CreateComputePipeline(fs);
	fwd_pipeline_->CreateShaderResourceBinding(fwd_srb_, false);
	delete fs;

	Shader* bs = LoadComputeShader("Shader/" + in_bwd_shader + ".spv");
	bwd_pipeline_ = CreateComputePipeline(bs);
	bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_, false);
	delete bs;
}

ActivationLayer::~ActivationLayer()
{
	if (bwd_srb_) delete bwd_srb_;
	if (fwd_srb_) delete fwd_srb_;
	//if (bwd_pipeline_) delete bwd_pipeline_;
	//if (fwd_pipeline_) delete fwd_pipeline_;
}

void ActivationLayer::Forward(CommandList* in_cmd, Tensor& in_input)
{
	fwd_srb_->SetResource("inp", in_input.GetBuffer());
	fwd_srb_->SetResource("out_buf", output_.GetBuffer());
	fwd_srb_->SetResource("saved", saved_input_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	fwd_srb_->FlushDescriptorWrites();

	in_cmd->SetComputePipeline(fwd_pipeline_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
}

void ActivationLayer::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& /*input_act*/)
{
	bwd_srb_->SetResource("dL_dout", in_dL_dout.GetBuffer());
	bwd_srb_->SetResource("dL_dx", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("saved", saved_input_.GetBuffer());
	bwd_srb_->SetResource("pc", pc_buf_.GetBuffer());
	bwd_srb_->FlushDescriptorWrites();

	in_cmd->SetComputePipeline(bwd_pipeline_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
}

} // namespace MXNN
