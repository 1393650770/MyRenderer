#include "ResidualBlock.h"
#include "ShaderHelper.h"
#include "Optimizer.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderPipelineState.h"
// --  

namespace MXNN {

// ======== ResidualBlock ========
ResidualBlock::ResidualBlock(UInt32 in_n_elem)
    : n_elem_(in_n_elem)
    , output_(Vector<UInt32>{in_n_elem})
    , dL_dx_(Vector<UInt32>{in_n_elem})
{
    Float32 pc[1] = { static_cast<Float32>(n_elem_) };
    pc_buf_.Upload(pc);
    Shader* s = LoadComputeShader("Shader/nn_add_tensors.comp.spv");
    add_pipeline_ = CreateComputePipeline(s);
    delete s;
}

ResidualBlock::~ResidualBlock() {
    for (auto* s : temp_srbs_) delete s;
}

void ResidualBlock::AddSubLayer(UniquePtr<ILayer> in_layer) {
    sub_layers_.push_back(std::move(in_layer));
}

void ResidualBlock::Forward(CommandList* in_cmd, Tensor& in_input) {
    if (sub_layers_.empty()) return;
    sub_layers_[0]->Forward(in_cmd, in_input);
    Tensor* cur = &sub_layers_[0]->GetOutput();
    for (size_t i = 1; i < sub_layers_.size(); ++i) {
        sub_layers_[i]->Forward(in_cmd, *cur);
        cur = &sub_layers_[i]->GetOutput();
    }
    ShaderResourceBinding* s = nullptr;
    add_pipeline_->CreateShaderResourceBinding(s, false);
    s->SetResource("a", cur->GetBuffer());
    s->SetResource("b", in_input.GetBuffer());
    s->SetResource("c", output_.GetBuffer());
    s->SetResource("pc", pc_buf_.GetBuffer());
    s->FlushDescriptorWrites();
    in_cmd->SetComputePipeline(add_pipeline_);
    in_cmd->SetShaderResourceBinding(s);
    in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
    temp_srbs_.push_back(s);
}

void ResidualBlock::Backward(CommandList* in_cmd, CONST Tensor& in_dL_dout, CONST Tensor& in_input_act) {
    if (sub_layers_.empty()) return;
    for (Int i = static_cast<Int>(sub_layers_.size()) - 1; i >= 0; --i) {
        CONST Tensor* g_in = (i == static_cast<Int>(sub_layers_.size()) - 1)
            ? &in_dL_dout : &sub_layers_[static_cast<size_t>(i) + 1]->GetInputGradient();
        CONST Tensor* act_in = (i > 0)
            ? &sub_layers_[static_cast<size_t>(i) - 1]->GetOutput() : &in_input_act;
        sub_layers_[i]->Backward(in_cmd, *g_in, *act_in);
    }
    ShaderResourceBinding* s = nullptr;
    add_pipeline_->CreateShaderResourceBinding(s, false);
    s->SetResource("a", in_dL_dout.GetBuffer());
    s->SetResource("b", sub_layers_[0]->GetInputGradient().GetBuffer());
    s->SetResource("c", dL_dx_.GetBuffer());
    s->SetResource("pc", pc_buf_.GetBuffer());
    s->FlushDescriptorWrites();
    in_cmd->SetComputePipeline(add_pipeline_);
    in_cmd->SetShaderResourceBinding(s);
    in_cmd->Dispatch((n_elem_ + 255u) / 256u, 1u, 1u);
    temp_srbs_.push_back(s);
}

void ResidualBlock::UpdateWeights(CommandList* in_cmd, IOptimizer& in_opt, Float32 in_inv_bs, UInt32 in_step) {
    for (auto& layer : sub_layers_) layer->UpdateWeights(in_cmd, in_opt, in_inv_bs, in_step);
}
void ResidualBlock::ZeroGradients(CommandList* in_cmd) {
    for (auto& layer : sub_layers_) layer->ZeroGradients(in_cmd);
}

Vector<std::tuple<Tensor*,Tensor*,Tensor*>> ResidualBlock::GetParamTriples() {
    Vector<std::tuple<Tensor*,Tensor*,Tensor*>> r;
    for (auto& layer : sub_layers_) {
        auto sub = layer->GetParamTriples();
        for (auto& t : sub) r.push_back(std::move(t));
    }
    return r;
}

// --   Persistence
void ResidualBlock::SaveParameters(std::ostream& os) const {
    os.write((char*)&n_elem_, 4);
    UInt32 count = (UInt32)sub_layers_.size();
    os.write((char*)&count, 4);
    for (auto& sl : sub_layers_) {
        String name = sl->GetLayerTypeName();
        UInt16 nlen = (UInt16)name.length();
        os.write((char*)&nlen, 2); os.write(name.data(), nlen);
        sl->SaveParameters(os);
    }
}
void ResidualBlock::LoadParameters(std::istream& is) {
    UInt32 count; is.read((char*)&count, 4);
    for (UInt32 i = 0; i < count; ++i) {
        UInt16 nlen; is.read((char*)&nlen, 2);
        String name(nlen, ' '); is.read(&name[0], nlen);
        if (i < sub_layers_.size()) sub_layers_[i]->LoadParameters(is);
    }
}
// --  

} // namespace MXNN
// --  
