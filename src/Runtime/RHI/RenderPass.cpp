#include "RenderPass.h"
#include"RenderState.h"
namespace MXRender
{
    void RenderPass::post_initialize()
    {
    }

    void RenderPass::set_commonInfo(const PassInfo& init_info)
    {
    }

    void RenderPass::prepare_pass_data(const GraphicsContext& context)
    {
    }

    void RenderPass::initialize_ui_renderbackend()
    {
    }


    MXRender::RenderPass::RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
