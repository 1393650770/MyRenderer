#include "Model.h"
#include <fstream>
#include <iostream>
#include <cstring>

#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"
using namespace MXRender::RHI; using namespace MXRender;
namespace MXNN {
static Shader* Ld(const char* fn){ShaderDesc d;d.shader_type=ENUM_SHADER_STAGE::Shader_Compute;d.entry_name="main";std::ifstream f(fn,std::ios::ate|std::ios::binary);if(!f.is_open()){std::cerr<<"ERR:"<<fn<<std::endl;return nullptr;}size_t sz=(size_t)f.tellg();Vector<UInt32> buf(sz/sizeof(UInt32));f.seekg(0);f.read((char*)buf.data(),sz);ShaderDataPayload p;p.data=std::move(buf);return RHICreateShader(d,p);}
static RenderPipelineState* Mk(Shader* cs){RenderGraphiPipelineStateDesc d{};d.shaders[ENUM_SHADER_STAGE::Shader_Compute]=cs;d.primitive_topology=ENUM_PRIMITIVE_TYPE::TriangleList;d.raster_state.sample_count=1;return RHICreateRenderPipelineState(d);}
SequentialModel::SequentialModel(uint32_t max_batch_size):max_batch_size_(max_batch_size),zg_pc_buf_({1}),label_buf_({max_batch_size}){Shader* zg=Ld("Shader/nn_zero_grad.comp.spv");zero_grad_pipeline_=Mk(zg);zero_grad_pipeline_->CreateShaderResourceBinding(zero_grad_srb_,false);zero_grad_srb_->SetResource("pcc",zg_pc_buf_.GetBuffer());delete zg;}
void SequentialModel::AddLayer(std::unique_ptr<ILayer> l){layers_.push_back(std::move(l));}
void SequentialModel::SetOptimizer(std::unique_ptr<IOptimizer> o){optimizer_=std::move(o);}
void SequentialModel::ZeroAllGradients(CommandList* cmd){ZeroParams zp;cmd->SetComputePipeline(zero_grad_pipeline_);cmd->SetShaderResourceBinding(zero_grad_srb_);for(auto& layer:layers_){for(auto&[p,g,v]:layer->GetParamTriples()){zp.num_elements=(uint32_t)g->ElementCount();zg_pc_buf_.Upload(&zp.num_elements);zero_grad_srb_->SetResource("g0",g->GetBuffer());cmd->Dispatch((zp.num_elements+255u)/256u,1,1);}}}
void SequentialModel::UpdateAllWeights(CommandList* cmd,float inv_bs,uint32_t step){for(auto& l:layers_)l->UpdateWeights(cmd,*optimizer_,inv_bs,step);}
float SequentialModel::TrainStep(CommandList* cmd, Tensor& input, const std::vector<uint8_t>& labels, uint32_t active_batch_size){
	auto* loss_layer=static_cast<SoftmaxCrossEntropyOutputLayer*>(layers_.back().get());
	std::vector<float> lf(active_batch_size);for(uint32_t i=0;i<active_batch_size;++i)lf[i]=(float)labels[i];label_buf_.Upload(lf.data());
	loss_layer->GetFwdLossSRB()->SetResource("lb",label_buf_.GetBuffer());
	auto* vk=static_cast<Vulkan::VK_CommandBuffer*>(cmd);
	ZeroAllGradients(cmd);
		{ZeroParams zp;zp.num_elements=1.0f;zg_pc_buf_.Upload(&zp.num_elements);zero_grad_srb_->SetResource("g0",loss_layer->GetLossBuffer());cmd->Dispatch(1,1,1);}
	vk->MemoryBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_ACCESS_SHADER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT);
	Tensor* cur=&input;for(size_t i=0;i<layers_.size();++i){layers_[i]->Forward(cmd,*cur);cur=&layers_[i]->GetOutput();}
	vk->MemoryBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_ACCESS_SHADER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT);
	for(int i=(int)layers_.size()-1;i>=0;--i){const Tensor* gi;const Tensor* ia;
		if(i==(int)layers_.size()-1){gi=&layers_[i]->GetOutput();ia=(i>0)?&layers_[i-1]->GetOutput():&input;}
		else{gi=&layers_[i+1]->GetInputGradient();ia=(i>0)?&layers_[i-1]->GetOutput():&input;}
		layers_[i]->Backward(cmd,*gi,*ia);}
	vk->MemoryBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_ACCESS_SHADER_WRITE_BIT,VK_ACCESS_SHADER_READ_BIT);
	float inv_bs=1.0f/(float)active_batch_size;UpdateAllWeights(cmd,inv_bs,step_count_++);
	return loss_layer->GetLoss()*inv_bs;
}
std::vector<uint8_t> SequentialModel::Predict(CommandList* cmd,Tensor& input,uint32_t batch_size){
	layers_[0]->Forward(cmd,input);Tensor& ho=layers_[0]->GetOutput();
	std::vector<float> zl(batch_size,0.0f);label_buf_.Upload(zl.data());
	auto* ll=static_cast<SoftmaxCrossEntropyOutputLayer*>(layers_.back().get());ll->GetFwdLossSRB()->SetResource("lb",label_buf_.GetBuffer());
	layers_[1]->Forward(cmd,ho);
	Tensor& probs=ll->GetOutput();std::vector<float> p(probs.ElementCount());probs.Download(p.data());
	uint32_t nc=ll->NumClasses();std::vector<uint8_t> preds(batch_size);
	for(uint32_t s=0;s<batch_size;++s){float best=-1e30f;uint8_t bc=0;for(uint32_t c=0;c<nc;++c){float v=p[s*nc+c];if(v>best){best=v;bc=(uint8_t)c;}}preds[s]=bc;}
	return preds;
}
} // namespace MXNN
