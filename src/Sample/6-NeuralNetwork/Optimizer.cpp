#include "Optimizer.h"
#include "Layer.h"
#include <fstream>
#include <iostream>
#include "RHI/RenderEnum.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderRource.h"
namespace MXNN {
using namespace MXRender::RHI; using namespace MXRender;
static Shader* Ld(const char* fn){ShaderDesc d;d.shader_type=ENUM_SHADER_STAGE::Shader_Compute;d.entry_name="main";std::ifstream f(fn,std::ios::ate|std::ios::binary);size_t sz=(size_t)f.tellg();Vector<UInt32> buf(sz/sizeof(UInt32));f.seekg(0);f.read((char*)buf.data(),sz);ShaderDataPayload p;p.data=std::move(buf);return RHICreateShader(d,p);}
static RenderPipelineState* Mk(Shader* cs){RenderGraphiPipelineStateDesc d{};d.shaders[ENUM_SHADER_STAGE::Shader_Compute]=cs;d.primitive_topology=ENUM_PRIMITIVE_TYPE::TriangleList;d.raster_state.sample_count=1;return RHICreateRenderPipelineState(d);}
SGD::SGD(float lr, float momentum, float weight_decay):lr_(lr),momentum_(momentum),weight_decay_(weight_decay),pc_buf_({5}){Shader* s=Ld("Shader/nn_update_sgd.comp.spv");update_pipeline_=Mk(s);update_pipeline_->CreateShaderResourceBinding(update_srb_,false);update_srb_->SetResource("pcc",pc_buf_.GetBuffer());delete s;}
SGD::~SGD(){if(update_srb_)delete update_srb_;if(update_pipeline_)delete update_pipeline_;}
void SGD::Update(CommandList* cmd, Tensor& params, Tensor& grads, Tensor& velocity, float inv_batch_size, uint32_t step){
	SgdParams p={lr_,momentum_,weight_decay_,inv_batch_size,(uint32_t)params.ElementCount()};pc_buf_.Upload(&p.lr);
	update_srb_->SetResource("p0",params.GetBuffer());update_srb_->SetResource("g0",grads.GetBuffer());update_srb_->SetResource("v0",velocity.GetBuffer());
	cmd->SetComputePipeline(update_pipeline_);cmd->SetShaderResourceBinding(update_srb_);cmd->Dispatch((p.n_elem+255u)/256u,1,1);(void)step;
}
} // namespace MXNN
