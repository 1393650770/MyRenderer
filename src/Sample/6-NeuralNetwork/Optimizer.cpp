#include "Optimizer.h"
#include "Layer.h"
#include <fstream>
#include <iostream>
#include "Core/ConstDefine.h"
#include "RHI/RenderEnum.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderRource.h"
#include "RHI/Vulkan/VK_Shader.h"

using namespace MXRender::RHI;
using namespace MXRender;

namespace MXNN {

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

SGD::SGD(Float32 in_lr, Float32 in_momentum, Float32 in_weight_decay)
	: lr_(in_lr), momentum_(in_momentum), weight_decay_(in_weight_decay), pc_buf_({5})
{
	Shader* shader = LoadComputeShader("Shader/nn_update_sgd.comp.spv");
	update_pipeline_ = CreateComputePipeline(shader);
	update_pipeline_->CreateShaderResourceBinding(update_srb_, false);
	update_srb_->SetResource("pcc", pc_buf_.GetBuffer());
	delete shader;
}

SGD::~SGD()
{
	if (update_srb_) delete update_srb_;
	if (update_pipeline_) delete update_pipeline_;
	for (auto* srb : temp_srbs_) delete srb;
}

void SGD::Update(CommandList* in_cmd, Tensor& in_params, Tensor& in_grads,
	Tensor& in_velocity, Float32 in_inv_batch_size, UInt32 /*in_step*/)
{
	SgdParams params;
	params.lr = lr_;
	params.momentum = momentum_;
	params.wd = weight_decay_;
	params.inv_bs = in_inv_batch_size;
	params.n_elem = static_cast<Float32>(in_params.ElementCount());
	pc_buf_.Upload(&params.lr);

	ShaderResourceBinding* temp_srb = nullptr;
	update_pipeline_->CreateShaderResourceBinding(temp_srb, false);
	temp_srb->SetResource("p0", in_params.GetBuffer());
	temp_srb->SetResource("g0", in_grads.GetBuffer());
	temp_srb->SetResource("v0", in_velocity.GetBuffer());
	temp_srb->SetResource("pcc", pc_buf_.GetBuffer());
	STATIC_CAST(temp_srb, Vulkan::VK_ShaderResourceBinding)->FlushDescriptorWrites();

	in_cmd->SetComputePipeline(update_pipeline_);
	in_cmd->SetShaderResourceBinding(temp_srb);
	in_cmd->Dispatch((static_cast<UInt32>(params.n_elem) + 255u) / 256u, 1, 1);

	temp_srbs_.push_back(temp_srb);
}

} // namespace MXNN
