#include "Layer.h"
#include "Optimizer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstring>
#include "Core/ConstDefine.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderCommandList.h"

using namespace MXRender::RHI;
using namespace MXRender;

namespace MXNN {

// ---- shared shader helpers (duplicated per-file per project convention) ----

static Vector<UInt32> ReadShader(CONST String& in_filename)
{
	std::ifstream file(in_filename, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!file.is_open(), " App Error: fail to open the shader file! ")

	size_t file_size = (size_t)file.tellg();
	Vector<UInt32> buffer(file_size / sizeof(UInt32));
	file.seekg(0);
	file.read((char*)buffer.data(), file_size);
	file.close();
	return std::move(buffer);
}

static Shader* LoadComputeShader(CONST String& in_filename)
{
	ShaderDesc desc;
	desc.shader_type = ENUM_SHADER_STAGE::Shader_Compute;
	desc.entry_name = "main";

	ShaderDataPayload payload;
	payload.data = ReadShader(in_filename);

	return RHICreateShader(desc, payload);
}

static RenderPipelineState* CreateComputePipeline(Shader* in_cs)
{
	RenderGraphiPipelineStateDesc desc{};
	desc.shaders[ENUM_SHADER_STAGE::Shader_Compute] = in_cs;
	desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	desc.raster_state.sample_count = 1;

	return RHICreateRenderPipelineState(desc);
}

// ============================================================
// LinearLayer
// ============================================================

LinearLayer::LinearLayer(UInt32 in_in_dim, UInt32 in_out_dim, UInt32 in_max_batch_size, Bool in_has_relu)
	: in_dim_(in_in_dim)
	, out_dim_(in_out_dim)
	, max_batch_size_(in_max_batch_size)
	, has_relu_(in_has_relu)
	, weight_({out_dim_, in_dim_})
	, bias_({out_dim_})
	, grad_w_({out_dim_, in_dim_})
	, grad_b_({out_dim_})
	, v_w_({out_dim_, in_dim_})
	, v_b_({out_dim_})
	, z_preact_({max_batch_size_, out_dim_})
	, output_({max_batch_size_, out_dim_})
	, dL_dx_({max_batch_size_, in_dim_})
	, pc_buf_({5})
{
	CreatePipelineAndSRB();
}

void LinearLayer::CreatePipelineAndSRB()
{
	// Forward pipeline & SRB
	Shader* fwd_shader = LoadComputeShader("Shader/nn_forward_linear.comp.spv");
	fwd_pipeline_ = CreateComputePipeline(fwd_shader);
	fwd_pipeline_->CreateShaderResourceBinding(fwd_srb_, false);

	fwd_srb_->SetResource("w0", weight_.GetBuffer());
	fwd_srb_->SetResource("b0", bias_.GetBuffer());
	fwd_srb_->SetResource("p0", z_preact_.GetBuffer());
	fwd_srb_->SetResource("a0", output_.GetBuffer());
	fwd_srb_->SetResource("pc", pc_buf_.GetBuffer());

	// Backward pipeline & SRB
	Shader* bwd_shader = LoadComputeShader("Shader/nn_backward_linear.comp.spv");
	bwd_pipeline_ = CreateComputePipeline(bwd_shader);
	bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_, false);

	bwd_srb_->SetResource("b2", weight_.GetBuffer());
	bwd_srb_->SetResource("b4", grad_w_.GetBuffer());
	bwd_srb_->SetResource("b5", grad_b_.GetBuffer());
	bwd_srb_->SetResource("b6", dL_dx_.GetBuffer());
	bwd_srb_->SetResource("b10", pc_buf_.GetBuffer());
		delete fwd_shader;
		delete bwd_shader;
}

void LinearLayer::Forward(CommandList* in_cmd, Tensor& in_input)
{
	FwdParams params;
	params.in_dim = static_cast<Float32>(in_dim_);
	params.out_dim = static_cast<Float32>(out_dim_);
	params.max_batch_size = static_cast<Float32>(max_batch_size_);
	params.active_batch_size = static_cast<Float32>(in_input.Shape()[0]);
	params.has_relu = has_relu_ ? 1.0f : 0.0f;
	pc_buf_.Upload(&params.in_dim);

	fwd_srb_->SetResource("i0", in_input.GetBuffer());

	in_cmd->SetComputePipeline(fwd_pipeline_);
	in_cmd->SetShaderResourceBinding(fwd_srb_);
	in_cmd->Dispatch(static_cast<UInt32>(params.max_batch_size), 1, 1);
}

void LinearLayer::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act)
{
	BwdParams params;
	params.in_dim = static_cast<Float32>(in_dim_);
	params.out_dim = static_cast<Float32>(out_dim_);
	params.max_batch_size = static_cast<Float32>(max_batch_size_);
	params.active_batch_size = static_cast<Float32>(in_input_act.Shape()[0]);
	params.has_relu = has_relu_ ? 1.0f : 0.0f;
	pc_buf_.Upload(&params.in_dim);

	bwd_srb_->SetResource("b0", in_input_act.GetBuffer());
	bwd_srb_->SetResource("b1", z_preact_.GetBuffer());
	bwd_srb_->SetResource("b3", in_dL_dout.GetBuffer());

	in_cmd->SetComputePipeline(bwd_pipeline_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch(static_cast<UInt32>(params.max_batch_size), 1, 1);
}

void LinearLayer::ZeroGradients(CommandList* /*in_cmd*/)
{
	// Handled by SequentialModel::ZeroAllGradients via zero_grad shader
}

void LinearLayer::UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step)
{
	in_opt.Update(in_cmd, weight_, grad_w_, v_w_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, bias_, grad_b_, v_b_, in_inv_bs, in_step);
}

Vector<std::tuple<Tensor*, Tensor*, Tensor*>> LinearLayer::GetParamTriples()
{
	return {
		{&weight_, &grad_w_, &v_w_},
		{&bias_,   &grad_b_, &v_b_}
	};
}

// ============================================================
// SoftmaxCrossEntropyOutputLayer
// ============================================================

SoftmaxCrossEntropyOutputLayer::SoftmaxCrossEntropyOutputLayer(
	UInt32 in_in_dim, UInt32 in_num_classes, UInt32 in_max_batch_size)
	: in_dim_(in_in_dim)
	, num_classes_(in_num_classes)
	, max_batch_size_(in_max_batch_size)
	, weight_({num_classes_, in_dim_})
	, bias_({num_classes_})
	, grad_w_({num_classes_, in_dim_})
	, grad_b_({num_classes_})
	, v_w_({num_classes_, in_dim_})
	, v_b_({num_classes_})
	, dL_dz_({max_batch_size_, num_classes_})
	, loss_buf_({1})
	, dL_dhidden_({max_batch_size_, in_dim_})
	, pc_buf_({4})
{
	// Forward + loss pipeline & SRB
	Shader* fwd_shader = LoadComputeShader("Shader/nn_forward_softmax_loss.comp.spv");
	fwd_loss_pipeline_ = CreateComputePipeline(fwd_shader);
	fwd_loss_pipeline_->CreateShaderResourceBinding(fwd_loss_srb_, false);

	fwd_loss_srb_->SetResource("w2", weight_.GetBuffer());
	fwd_loss_srb_->SetResource("b2", bias_.GetBuffer());
	fwd_loss_srb_->SetResource("dz", dL_dz_.GetBuffer());
	fwd_loss_srb_->SetResource("ls", loss_buf_.GetBuffer());
	fwd_loss_srb_->SetResource("pc", pc_buf_.GetBuffer());

	// Backward pipeline & SRB (load BEFORE deleting fwd_shader to avoid pointer reuse)
	Shader* bwd_shader = LoadComputeShader("Shader/nn_backward_linear.comp.spv");
	bwd_pipeline_ = CreateComputePipeline(bwd_shader);
	bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_, false);

	bwd_srb_->SetResource("b2", weight_.GetBuffer());
	bwd_srb_->SetResource("b4", grad_w_.GetBuffer());
	bwd_srb_->SetResource("b5", grad_b_.GetBuffer());
	bwd_srb_->SetResource("b6", dL_dhidden_.GetBuffer());
	bwd_srb_->SetResource("b10", pc_buf_.GetBuffer());

	// Now safe to delete both shaders
	delete fwd_shader;
	delete bwd_shader;
}

void SoftmaxCrossEntropyOutputLayer::Forward(CommandList* in_cmd, Tensor& in_hidden_act)
{
	LossParams params;
	params.in_dim = static_cast<Float32>(in_dim_);
	params.out_dim = static_cast<Float32>(num_classes_);
	params.max_batch_size = static_cast<Float32>(max_batch_size_);
	params.active_batch_size = static_cast<Float32>(in_hidden_act.Shape()[0]);
	pc_buf_.Upload(&params.in_dim);

	fwd_loss_srb_->SetResource("h0", in_hidden_act.GetBuffer());

	in_cmd->SetComputePipeline(fwd_loss_pipeline_);
	in_cmd->SetShaderResourceBinding(fwd_loss_srb_);
	in_cmd->Dispatch(static_cast<UInt32>(params.max_batch_size), 1, 1);
}

void SoftmaxCrossEntropyOutputLayer::Backward(
	CommandList* in_cmd, CONST Tensor& /*in_dL_dout*/, CONST Tensor& in_input_act)
{
	BwdParams params;
	params.in_dim = static_cast<Float32>(in_dim_);
	params.out_dim = static_cast<Float32>(num_classes_);
	params.max_batch_size = static_cast<Float32>(max_batch_size_);
	params.active_batch_size = static_cast<Float32>(max_batch_size_);
	params.has_relu = 0.0f;
	pc_buf_.Upload(&params.in_dim);

	bwd_srb_->SetResource("b0", in_input_act.GetBuffer());
	bwd_srb_->SetResource("b1", in_input_act.GetBuffer()); // preact unused (ReLU off)
	bwd_srb_->SetResource("b3", dL_dz_.GetBuffer());

	in_cmd->SetComputePipeline(bwd_pipeline_);
	in_cmd->SetShaderResourceBinding(bwd_srb_);
	in_cmd->Dispatch(static_cast<UInt32>(params.max_batch_size), 1, 1);
}

void SoftmaxCrossEntropyOutputLayer::ZeroGradients(CommandList* /*in_cmd*/)
{
	// Handled by SequentialModel::ZeroAllGradients
}

void SoftmaxCrossEntropyOutputLayer::UpdateWeights(
	CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step)
{
	in_opt.Update(in_cmd, weight_, grad_w_, v_w_, in_inv_bs, in_step);
	in_opt.Update(in_cmd, bias_, grad_b_, v_b_, in_inv_bs, in_step);
}

Float32 SoftmaxCrossEntropyOutputLayer::GetLoss() CONST
{
	Float32 loss = 0.0f;
	loss_buf_.Download(&loss);
	return loss;
}

Vector<std::tuple<Tensor*, Tensor*, Tensor*>> SoftmaxCrossEntropyOutputLayer::GetParamTriples()
{
	return {
		{&weight_, &grad_w_, &v_w_},
		{&bias_,   &grad_b_, &v_b_}
	};
}

} // namespace MXNN
