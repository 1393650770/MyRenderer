#include "Model.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include "Core/ConstDefine.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "Normalization.h"
#include "Activation.h"
#include "ConvLayer.h"
#include "AttentionLayer.h"
#include "ResidualBlock.h"
#include "ShaderHelper.h"

using namespace MXRender::RHI;
using namespace MXRender;

namespace MXNN {

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
	zero_grad_srb_->SetResource("pc", zg_pc_buf_.GetBuffer());
	delete zg_shader;
}

SequentialModel::~SequentialModel()
{
	if (zero_grad_srb_) delete zero_grad_srb_;
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
			temp_srb->SetResource("pc", zg_pc_buf_.GetBuffer());

			// Flush writes BEFORE binding (update→bind order is valid)
			temp_srb->FlushDescriptorWrites();

			in_cmd->SetShaderResourceBinding(temp_srb);
			in_cmd->Dispatch(
				(static_cast<UInt32>(zp.num_elements) + 255u) / 256u, 1, 1);

			// Defer deletion to avoid freeing while CB is recording
			zg_temp_srbs_.push_back(temp_srb);
		}
		for (auto& q : layer->GetParamQuads()) {
			ShaderResourceBinding* ts = nullptr;
			zero_grad_pipeline_->CreateShaderResourceBinding(ts, false);
			zp.num_elements = static_cast<Float32>(std::get<3>(q)->ElementCount());
			zg_pc_buf_.Upload(&zp.num_elements);
			ts->SetResource("g0", std::get<3>(q)->GetBuffer());
			ts->SetResource("pc", zg_pc_buf_.GetBuffer());
			ts->FlushDescriptorWrites();
			in_cmd->SetShaderResourceBinding(ts);
			in_cmd->Dispatch((static_cast<UInt32>(zp.num_elements) + 255u) / 256u, 1, 1);
			zg_temp_srbs_.push_back(ts);
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

	
	// 1. Zero all gradients (GPU compute)
	for (auto& layer : layers_) layer->SetTrainingMode(true);
	ZeroAllGradients(in_cmd);

	// 2. Zero loss buffer (GPU compute, reuse zero_grad pipeline)
	{
		ShaderResourceBinding* temp_srb = nullptr;
		zero_grad_pipeline_->CreateShaderResourceBinding(temp_srb, false);

		ZeroParams zp;
		zp.num_elements = 1.0f;
		zg_pc_buf_.Upload(&zp.num_elements);
		temp_srb->SetResource("g0", loss_layer->GetLossBuffer());
		temp_srb->SetResource("pc", zg_pc_buf_.GetBuffer());
		temp_srb->FlushDescriptorWrites();

		in_cmd->SetShaderResourceBinding(temp_srb);
		in_cmd->Dispatch(1, 1, 1);
		zg_temp_srbs_.push_back(temp_srb);
	}

	// Barrier: zero writes → forward reads
	in_cmd->MemoryBarrier(
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_RESOURCE_STATE::UnorderedAccess,
		ENUM_RESOURCE_STATE::ShaderResource);

	// 3. Forward pass: input → hidden → output (with loss)
	Tensor* current = &in_input;
	for (size_t i = 0; i < layers_.size(); ++i)
	{
		layers_[i]->Forward(in_cmd, *current);
		current = &layers_[i]->GetOutput();
	}

	// Barrier: forward writes → backward reads
	in_cmd->MemoryBarrier(
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_RESOURCE_STATE::UnorderedAccess,
		ENUM_RESOURCE_STATE::ShaderResource);

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
	in_cmd->MemoryBarrier(
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_SHADER_STAGE::Shader_Compute,
		ENUM_RESOURCE_STATE::UnorderedAccess,
		ENUM_RESOURCE_STATE::ShaderResource);

	// 5. Weight update
	Float32 inv_bs = 1.0f / static_cast<Float32>(in_active_batch_size);
	UpdateAllWeights(in_cmd, inv_bs, step_count_++);

	return loss_layer->GetLoss() * inv_bs;
}

Vector<UInt8> SequentialModel::Predict(CommandList* in_cmd, Tensor& in_input, UInt32 in_batch_size)
{
	PredictForward(in_cmd, in_input);
	return GetPredictions(in_batch_size);
}

void SequentialModel::PredictForward(CommandList* in_cmd, Tensor& in_input)
{
	// Bind zero labels for SoftmaxCE inference
	{ Vector<Float32> zeros(max_batch_size_, 0.0f); label_buf_.Upload(zeros.data()); }
	for (auto& layer : layers_) { layer->SetTrainingMode(false); if (layer->GetLayerTypeName() == "SoftmaxCrossEntropyOutputLayer") { STATIC_CAST(layer.get(), SoftmaxCrossEntropyOutputLayer)->GetFwdLossSRB()->SetResource("lb", label_buf_.GetBuffer()); } }
	Tensor* current = &in_input;
	for (size_t i = 0; i < layers_.size(); ++i)
	{
		layers_[i]->Forward(in_cmd, *current);
		current = &layers_[i]->GetOutput();
	}
}

Vector<UInt8> SequentialModel::GetPredictions(UInt32 in_batch_size)
{
	Tensor& probs = layers_.back()->GetOutput();
	Vector<Float32> prob_data(probs.ElementCount());
	probs.Download(prob_data.data());

	UInt32 num_classes = probs.Shape().back();
	// Output rows: CNN [B,C] one row per sample; Transformer [B*T,C] one row per token
	UInt32 num_output_rows = static_cast<UInt32>(probs.ElementCount()) / num_classes;
	Vector<UInt8> predictions(in_batch_size);

	// rows_per_sample = 1 (no pooling needed); >1 (average pool tokens then argmax)
	UInt32 rows_per_sample = (std::max)(1u, num_output_rows / in_batch_size);

	for (UInt32 s = 0; s < in_batch_size; ++s)
	{
		Float32 best = -1e30f;
		UInt8 best_class = 0;
		UInt32 base_row = s * rows_per_sample;
		for (UInt32 c = 0; c < num_classes; ++c)
		{
			// Average pool across all tokens for this sample
			Float32 avg_prob = 0.0f;
			for (UInt32 t = 0; t < rows_per_sample; ++t)
			{
				UInt32 idx = (base_row + t) * num_classes + c;
				if (idx < prob_data.size()) // bounds check
					avg_prob += prob_data[idx];
			}
			avg_prob /= static_cast<Float32>(rows_per_sample);
			if (avg_prob > best)
			{
				best = avg_prob;
				best_class = static_cast<UInt8>(c);
			}
		}
		predictions[s] = best_class;
	}

	return predictions;
}

void SequentialModel::Save(CONST String& in_filepath) {
	std::ofstream ofs(in_filepath, std::ios::binary);
	if (!ofs) return;
	UInt32 magic = 0x01454E4Eu;
	UInt32 version = 1u;
	UInt32 count = (UInt32)layers_.size();
	ofs.write((char*)&magic, 4); ofs.write((char*)&version, 4); ofs.write((char*)&count, 4);
	for (auto& layer : layers_) {
		String name = layer->GetLayerTypeName();
		UInt16 nlen = (UInt16)name.length();
		ofs.write((char*)&nlen, 2); ofs.write(name.data(), nlen);
		layer->SaveParameters(ofs);
	}
}
void SequentialModel::Load(CONST String& in_filepath) {
	std::ifstream ifs(in_filepath, std::ios::binary);
	if (!ifs) return;
	UInt32 magic, version, count;
	ifs.read((char*)&magic, 4); ifs.read((char*)&version, 4); ifs.read((char*)&count, 4);
	if (magic != 0x01454E4Eu) return;
	layers_.clear();
	for (UInt32 i = 0; i < count; ++i) {
		UInt16 nlen; ifs.read((char*)&nlen, 2);
		String name(nlen, ' '); ifs.read(&name[0], nlen);
		if (name == "LinearLayer") {
			UInt32 a,b,c; Bool d; ifs.read((char*)&a,4);ifs.read((char*)&b,4);ifs.read((char*)&c,4);ifs.read((char*)&d,1);
			auto l = std::make_unique<LinearLayer>(a,b,c,d); l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "SoftmaxCrossEntropyOutputLayer") {
			UInt32 a,b,c; ifs.read((char*)&a,4);ifs.read((char*)&b,4);ifs.read((char*)&c,4);
			auto l = std::make_unique<SoftmaxCrossEntropyOutputLayer>(a,b,c); l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "LayerNorm") {
			UInt32 nf,mb; Float32 e; ifs.read((char*)&nf,4);ifs.read((char*)&mb,4);ifs.read((char*)&e,4);
			auto l = std::make_unique<LayerNorm>(nf,mb,e); l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "BatchNorm1D") {
			UInt32 nf,mb; Float32 mo,e; ifs.read((char*)&nf,4);ifs.read((char*)&mb,4);ifs.read((char*)&mo,4);ifs.read((char*)&e,4);
			auto l = std::make_unique<BatchNorm1DLayer>(nf,mb,mo,e); l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "Dropout") {
			Float32 p; UInt32 ne; ifs.read((char*)&p,4);ifs.read((char*)&ne,4);
			auto l = std::make_unique<DropoutLayer>(p,ne); l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "Conv2DLayer") {
			UInt32 in_c,out_c,k_h,k_w,in_h,in_w,mb,s,p;
			ifs.read((char*)&in_c,4);ifs.read((char*)&out_c,4);
			ifs.read((char*)&k_h,4);ifs.read((char*)&k_w,4);
			ifs.read((char*)&in_h,4);ifs.read((char*)&in_w,4);
			ifs.read((char*)&mb,4);ifs.read((char*)&s,4);ifs.read((char*)&p,4);
			auto l = std::make_unique<Conv2DLayer>(in_c,out_c,k_h,k_w,in_h,in_w,mb,s,p);
			l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "MultiHeadAttention") {
			UInt32 dm,nh,mb,ml;
			ifs.read((char*)&dm,4);ifs.read((char*)&nh,4);
			ifs.read((char*)&mb,4);ifs.read((char*)&ml,4);
			auto l = std::make_unique<MultiHeadAttentionLayer>(dm,nh,mb,ml);
			l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "ResidualBlock") {
			UInt32 ne; ifs.read((char*)&ne,4);
			auto l = std::make_unique<ResidualBlock>(ne);
			l->LoadParameters(ifs); layers_.push_back(std::move(l));
		} else if (name == "ReLU") {
			UInt32 r; ifs.read((char*)&r,4); Vector<UInt32> sh(r); ifs.read((char*)sh.data(),r*4);
			UInt32 ne=1; for(auto d:sh) ne*=d;
			auto l = std::make_unique<ReLULayer>(sh); layers_.push_back(std::move(l));
		} else if (name == "LeakyReLU" || name == "Sigmoid" || name == "Tanh" || name == "GELU" || name == "SiLU") {
			UInt32 r; ifs.read((char*)&r,4); Vector<UInt32> sh(r); ifs.read((char*)sh.data(),r*4);
			UniquePtr<ILayer> l;
			if (name == "LeakyReLU") l = std::make_unique<LeakyReLULayer>(sh);
			else if (name == "Sigmoid") l = std::make_unique<SigmoidLayer>(sh);
			else if (name == "Tanh") l = std::make_unique<TanhLayer>(sh);
			else if (name == "GELU") l = std::make_unique<GELULayer>(sh);
			else l = std::make_unique<SiLULayer>(sh);
			layers_.push_back(std::move(l));
		}
	}
}


void SequentialModel::ClearTempSRBs() {
	for (auto& layer : layers_) layer->ClearTempSRBs();
}
} // namespace MXNN
