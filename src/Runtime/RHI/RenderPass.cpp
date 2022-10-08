#include "RenderPass.h"
#include"RenderState.h"
#include "GraphicsContext.h"
#include "Vulkan/VK_Viewport.h"
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


	void RenderPass::draw(GraphicsContext* context, uint32_t& image_index, VK_Viewport* viewport)
	{
        
	}

	std::weak_ptr<MXRender::GraphicsContext> RenderPass::get_context()
	{
        return cur_context;
	}

	RenderPass::RenderPass()
	{

	}

	MXRender::RenderPass::RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
