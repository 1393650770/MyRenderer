#include "Layer.h"
#include "Optimizer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstring>
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderCommandList.h"
namespace MXNN {
using namespace MXRender::RHI; using namespace MXRender;
static Vector<UInt32> ReadSPV(const char* fn){std::ifstream f(fn,std::ios::ate|std::ios::binary);if(!f.is_open()){std::cerr<<"ERR:"<<fn<<std::endl;return{};}size_t sz=(size_t)f.tellg();Vector<UInt32> buf(sz/sizeof(UInt32));f.seekg(0);f.read((char*)buf.data(),sz);return buf;}
static Shader* Ld(const char* fn){ShaderDesc d;d.shader_type=ENUM_SHADER_STAGE::Shader_Compute;d.entry_name="main";ShaderDataPayload p;p.data=ReadSPV(fn);return RHICreateShader(d,p);}
static RenderPipelineState* Mk(Shader* cs){RenderGraphiPipelineStateDesc d{};d.shaders[ENUM_SHADER_STAGE::Shader_Compute]=cs;d.primitive_topology=ENUM_PRIMITIVE_TYPE::TriangleList;d.raster_state.sample_count=1;return RHICreateRenderPipelineState(d);}

// ====== LinearLayer ======
LinearLayer::LinearLayer(uint32_t in_dim, uint32_t out_dim, uint32_t max_batch_size, bool has_relu)
	: in_dim_(in_dim), out_dim_(out_dim), max_batch_size_(max_batch_size), has_relu_(has_relu)
	, weight_({out_dim, in_dim}), bias_({out_dim}), grad_w_({out_dim, in_dim}), grad_b_({out_dim})
	, v_w_({out_dim, in_dim}), v_b_({out_dim}), z_preact_({max_batch_size, out_dim}), output_({max_batch_size, out_dim})
	, dL_dx_({max_batch_size, in_dim}), pc_buf_({5}) { CreatePipelineAndSRB(); }
LinearLayer::~LinearLayer(){if(fwd_srb_)delete fwd_srb_;if(bwd_srb_)delete bwd_srb_;if(fwd_pipeline_)delete fwd_pipeline_;if(bwd_pipeline_)delete bwd_pipeline_;}
void LinearLayer::CreatePipelineAndSRB(){
	Shader* fs=Ld("Shader/nn_forward_linear.comp.spv");fwd_pipeline_=Mk(fs);fwd_pipeline_->CreateShaderResourceBinding(fwd_srb_,false);
	fwd_srb_->SetResource("w0",weight_.GetBuffer());fwd_srb_->SetResource("b0",bias_.GetBuffer());
	fwd_srb_->SetResource("p0",z_preact_.GetBuffer());fwd_srb_->SetResource("a0",output_.GetBuffer());
	fwd_srb_->SetResource("pc",pc_buf_.GetBuffer());delete fs;
	Shader* bs=Ld("Shader/nn_backward_linear.comp.spv");bwd_pipeline_=Mk(bs);bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_,false);
	bwd_srb_->SetResource("b2",weight_.GetBuffer());bwd_srb_->SetResource("b4",grad_w_.GetBuffer());
	bwd_srb_->SetResource("b5",grad_b_.GetBuffer());bwd_srb_->SetResource("b6",dL_dx_.GetBuffer());
	bwd_srb_->SetResource("b10",pc_buf_.GetBuffer());delete bs;
}
void LinearLayer::Forward(CommandList* cmd, Tensor& input){
	FwdParams p={in_dim_,out_dim_,max_batch_size_,(uint32_t)input.Shape()[0],has_relu_?1u:0u};pc_buf_.Upload(&p.in_dim);
	fwd_srb_->SetResource("i0",input.GetBuffer());
	cmd->SetComputePipeline(fwd_pipeline_);cmd->SetShaderResourceBinding(fwd_srb_);cmd->Dispatch(p.max_batch_size,1,1);
}
void LinearLayer::Backward(CommandList* cmd, const Tensor& dL_dout, const Tensor& input_act){
	BwdParams p={in_dim_,out_dim_,max_batch_size_,(uint32_t)input_act.Shape()[0],has_relu_?1u:0u};pc_buf_.Upload(&p.in_dim);
	bwd_srb_->SetResource("b0",input_act.GetBuffer());bwd_srb_->SetResource("b1",z_preact_.GetBuffer());
	bwd_srb_->SetResource("b3",dL_dout.GetBuffer());
	cmd->SetComputePipeline(bwd_pipeline_);cmd->SetShaderResourceBinding(bwd_srb_);cmd->Dispatch(p.max_batch_size,1,1);
}
void LinearLayer::ZeroGradients(CommandList*){}
void LinearLayer::UpdateWeights(CommandList* cmd, IOptimizer& opt, float inv_bs, uint32_t step){opt.Update(cmd,weight_,grad_w_,v_w_,inv_bs,step);opt.Update(cmd,bias_,grad_b_,v_b_,inv_bs,step);}
std::vector<std::tuple<Tensor*,Tensor*,Tensor*>> LinearLayer::GetParamTriples(){return{{&weight_,&grad_w_,&v_w_},{&bias_,&grad_b_,&v_b_}};}

// ====== SoftmaxCrossEntropyOutputLayer ======
SoftmaxCrossEntropyOutputLayer::SoftmaxCrossEntropyOutputLayer(uint32_t in_dim, uint32_t num_classes, uint32_t max_batch_size)
	: in_dim_(in_dim), num_classes_(num_classes), max_batch_size_(max_batch_size)
	, weight_({num_classes, in_dim}), bias_({num_classes}), grad_w_({num_classes, in_dim}), grad_b_({num_classes})
	, v_w_({num_classes, in_dim}), v_b_({num_classes}), dL_dz_({max_batch_size, num_classes}), loss_buf_({1})
	, dL_dhidden_({max_batch_size, in_dim}), pc_buf_({4})
{
	Shader* fs=Ld("Shader/nn_forward_softmax_loss.comp.spv");fwd_loss_pipeline_=Mk(fs);fwd_loss_pipeline_->CreateShaderResourceBinding(fwd_loss_srb_,false);
	fwd_loss_srb_->SetResource("w2",weight_.GetBuffer());fwd_loss_srb_->SetResource("b2",bias_.GetBuffer());
	fwd_loss_srb_->SetResource("dz",dL_dz_.GetBuffer());fwd_loss_srb_->SetResource("ls",loss_buf_.GetBuffer());fwd_loss_srb_->SetResource("pc",pc_buf_.GetBuffer());delete fs;
	Shader* bs=Ld("Shader/nn_backward_linear.comp.spv");bwd_pipeline_=Mk(bs);bwd_pipeline_->CreateShaderResourceBinding(bwd_srb_,false);
	bwd_srb_->SetResource("b2",weight_.GetBuffer());bwd_srb_->SetResource("b4",grad_w_.GetBuffer());
	bwd_srb_->SetResource("b5",grad_b_.GetBuffer());bwd_srb_->SetResource("b6",dL_dhidden_.GetBuffer());
	bwd_srb_->SetResource("b10",pc_buf_.GetBuffer());delete bs;
}
SoftmaxCrossEntropyOutputLayer::~SoftmaxCrossEntropyOutputLayer(){if(fwd_loss_srb_)delete fwd_loss_srb_;if(bwd_srb_)delete bwd_srb_;if(fwd_loss_pipeline_)delete fwd_loss_pipeline_;if(bwd_pipeline_)delete bwd_pipeline_;}
void SoftmaxCrossEntropyOutputLayer::Forward(CommandList* cmd, Tensor& hidden_act){
	LossParams p={in_dim_,num_classes_,max_batch_size_,(uint32_t)hidden_act.Shape()[0]};pc_buf_.Upload(&p.in_dim);
	fwd_loss_srb_->SetResource("h0",hidden_act.GetBuffer());
	cmd->SetComputePipeline(fwd_loss_pipeline_);cmd->SetShaderResourceBinding(fwd_loss_srb_);cmd->Dispatch(p.max_batch_size,1,1);
}
void SoftmaxCrossEntropyOutputLayer::Backward(CommandList* cmd, const Tensor&, const Tensor& input_act){
	BwdParams p={in_dim_,num_classes_,max_batch_size_,max_batch_size_,0u};pc_buf_.Upload(&p.in_dim);
	bwd_srb_->SetResource("b0",input_act.GetBuffer());bwd_srb_->SetResource("b1",input_act.GetBuffer());
	bwd_srb_->SetResource("b3",dL_dz_.GetBuffer());
	cmd->SetComputePipeline(bwd_pipeline_);cmd->SetShaderResourceBinding(bwd_srb_);cmd->Dispatch(p.max_batch_size,1,1);
}
void SoftmaxCrossEntropyOutputLayer::ZeroGradients(CommandList*){}
void SoftmaxCrossEntropyOutputLayer::UpdateWeights(CommandList* cmd, IOptimizer& opt, float inv_bs, uint32_t step){opt.Update(cmd,weight_,grad_w_,v_w_,inv_bs,step);opt.Update(cmd,bias_,grad_b_,v_b_,inv_bs,step);}
float SoftmaxCrossEntropyOutputLayer::GetLoss()const{float l=0;loss_buf_.Download(&l);return l;}
std::vector<std::tuple<Tensor*,Tensor*,Tensor*>> SoftmaxCrossEntropyOutputLayer::GetParamTriples(){return{{&weight_,&grad_w_,&v_w_},{&bias_,&grad_b_,&v_b_}};}
} // namespace MXNN
