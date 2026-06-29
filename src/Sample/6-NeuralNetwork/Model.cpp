#include "Model.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include "Core/ConstDefine.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"
#include "RHI/Vulkan/VK_Shader.h"

using namespace MXRender::RHI;
using namespace MXRender;

namespace MXNN {

// ---- shared shader helpers ----

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
// SequentialModel
// ============================================================

SequentialModel::SequentialModel(UInt32 in_max_batch_size)
	: max_batch_size_(in_max_batch_size)
	, zg_pc_buf_({1})
	, label_buf_({in_max_batch_size})
{
	Shader* zg_shader = LoadComputeShader("Shader/nn_zero_grad.comp.spv");
	zero_grad_pipeline_ = CreateComputePipeline(zg_shader);
	zero_grad_pipeline_->CreateShaderResourceBinding(zero_grad_srb_, false);
	zero_grad_srb_->SetResource("pcc", zg_pc_buf_.GetBuffer());
	delete zg_shader;
}

SequentialModel::~SequentialModel()
{
	if (zero_grad_srb_) delete zero_grad_srb_;
	if (zero_grad_pipeline_) delete zero_grad_pipeline_;
	for (auto* srb : zg_temp_srbs_) delete srb;
}

void SequentialModel::AddLayer(UniquePtr<ILayer> in_layer)
{
	layers_.push_back(std::move(in_layer));
}

void SequentialModel::SetOptimizer(UniquePtr<IOptimizer> in_opt)
{
	optimizer_ = std::move(in_opt);
}

void SequentialModel::ZeroAllGradients(CommandList* in_cmd)
{
	ZeroParams zp;
	in_cmd->SetComputePipeline(zero_grad_pipeline_);

	for (auto& layer : layers_)
	{
		for (auto& [params, grads, velocity] : layer->GetParamTriples())
		{
			// Fresh SRB per dispatch: avoids vkUpdateDescriptorSets
			// on a descriptor set already bound in this recording.
			ShaderResourceBinding* temp_srb = nullptr;
			zero_grad_pipeline_->CreateShaderResourceBinding(temp_srb, false);

			zp.num_elements = static_cast<Float32>(grads->ElementCount());
			zg_pc_buf_.Upload(&zp.num_elements);

			temp_srb->SetResource("g0", grads->GetBuffer());
			temp_srb->SetResource("pcc", zg_pc_buf_.GetBuffer());

			// Flush writes BEFORE binding (update→bind order is valid)
			STATIC_CAST(temp_srb, Vulkan::VK_ShaderResourceBinding)->FlushDescriptorWrites();

			in_cmd->SetShaderResourceBinding(temp_srb);
			in_cmd->Dispatch(
				(static_cast<UInt32>(zp.num_elements) + 255u) / 256u, 1, 1);

			// Defer deletion to avoid freeing while CB is recording
			zg_temp_srbs_.push_back(temp_srb);
		}
	}
}

void SequentialModel::UpdateAllWeights(CommandList* in_cmd, Float32 in_inv_bs, UInt32 in_step)
{
	for (auto& layer : layers_)
	{
		layer->UpdateWeights(in_cmd, *optimizer_, in_inv_bs, in_step);
	}
}

Float32 SequentialModel::TrainStep(CommandList* in_cmd, Tensor& in_input,
	CONST Vector<UInt8>& in_labels, UInt32 in_active_batch_size)
{
	auto* loss_layer = STATIC_CAST(layers_.back().get(), SoftmaxCrossEntropyOutputLayer);

	// Upload labels to GPU
	Vector<Float32> label_floats(in_active_batch_size);
	for (UInt32 i = 0; i < in_active_batch_size; ++i)
	{
		label_floats[i] = static_cast<Float32>(in_labels[i]);
	}
	label_buf_.Upload(label_floats.data());
	loss_layer->GetFwdLossSRB()->SetResource("lb", label_buf_.GetBuffer());

	auto* vk_cmd = STATIC_CAST(in_cmd, Vulkan::VK_CommandBuffer);

	// 1. Zero all gradients (GPU compute)
	ZeroAllGradients(in_cmd);

	// 2. Zero loss buffer (GPU compute, reuse zero_grad pipeline)
	{
		ShaderResourceBinding* temp_srb = nullptr;
		zero_grad_pipeline_->CreateShaderResourceBinding(temp_srb, false);

		ZeroParams zp;
		zp.num_elements = 1.0f;
		zg_pc_buf_.Upload(&zp.num_elements);
		temp_srb->SetResource("g0", loss_layer->GetLossBuffer());
		temp_srb->SetResource("pcc", zg_pc_buf_.GetBuffer());
		STATIC_CAST(temp_srb, Vulkan::VK_ShaderResourceBinding)->FlushDescriptorWrites();

		in_cmd->SetShaderResourceBinding(temp_srb);
		in_cmd->Dispatch(1, 1, 1);
		zg_temp_srbs_.push_back(temp_srb);
	}

	// Barrier: zero writes → forward reads
	vk_cmd->MemoryBarrier(
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT);

	// 3. Forward pass: input → hidden → output (with loss)
	Tensor* current = &in_input;
	for (size_t i = 0; i < layers_.size(); ++i)
	{
		layers_[i]->Forward(in_cmd, *current);
		current = &layers_[i]->GetOutput();
	}

	// Barrier: forward writes → backward reads
	vk_cmd->MemoryBarrier(
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT);

	// 4. Backward pass: output → hidden → input
	for (Int i = static_cast<Int>(layers_.size()) - 1; i >= 0; --i)
	{
		CONST Tensor* gradient_input = nullptr;
		CONST Tensor* input_activation = nullptr;

		if (i == static_cast<Int>(layers_.size()) - 1)
		{
			gradient_input = &layers_[i]->GetOutput();
			input_activation = (i > 0)
				? &layers_[static_cast<size_t>(i) - 1]->GetOutput()
				: &in_input;
		}
		else
		{
			gradient_input = &layers_[static_cast<size_t>(i) + 1]->GetInputGradient();
			input_activation = (i > 0)
				? &layers_[static_cast<size_t>(i) - 1]->GetOutput()
				: &in_input;
		}

		layers_[i]->Backward(in_cmd, *gradient_input, *input_activation);
	}

	// Barrier: backward writes → optimizer reads
	vk_cmd->MemoryBarrier(
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT);

	// 5. Weight update
	Float32 inv_bs = 1.0f / static_cast<Float32>(in_active_batch_size);
	UpdateAllWeights(in_cmd, inv_bs, step_count_++);

	return loss_layer->GetLoss() * inv_bs;
}

Vector<UInt8> SequentialModel::Predict(CommandList* in_cmd, Tensor& in_input, UInt32 in_batch_size)
{
	layers_[0]->Forward(in_cmd, in_input);
	Tensor& hidden_output = layers_[0]->GetOutput();

	Vector<Float32> zero_labels(in_batch_size, 0.0f);
	label_buf_.Upload(zero_labels.data());
	auto* loss_layer = STATIC_CAST(layers_.back().get(), SoftmaxCrossEntropyOutputLayer);
	loss_layer->GetFwdLossSRB()->SetResource("lb", label_buf_.GetBuffer());

	layers_[1]->Forward(in_cmd, hidden_output);

	Tensor& probs = loss_layer->GetOutput();
	Vector<Float32> prob_data(probs.ElementCount());
	probs.Download(prob_data.data());

	UInt32 num_classes = loss_layer->NumClasses();
	Vector<UInt8> predictions(in_batch_size);

	for (UInt32 s = 0; s < in_batch_size; ++s)
	{
		Float32 best = -1e30f;
		UInt8 best_class = 0;
		for (UInt32 c = 0; c < num_classes; ++c)
		{
			Float32 value = prob_data[s * num_classes + c];
			if (value > best)
			{
				best = value;
				best_class = static_cast<UInt8>(c);
			}
		}
		predictions[s] = best_class;
	}

	return predictions;
}

} // namespace MXNN
