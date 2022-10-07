#include "VK_RenderPass.h"
#include "VK_Viewport.h"

namespace MXRender
{

	void VK_RenderPass::initialize(const PassInfo& init_info, std::shared_ptr<GraphicsContext> context)
	{
		
	}

	void VK_RenderPass::initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context, std::weak_ptr<VK_Viewport> viewport)
	{

	}

	void VK_RenderPass::post_initialize()
	{

	}

	void VK_RenderPass::set_commonInfo(const PassInfo& init_info)
	{

	}

	void VK_RenderPass::prepare_pass_data()
	{

	}

	void VK_RenderPass::initialize_ui_renderbackend()
	{

	}

	VK_RenderPass::VK_RenderPass(const PassInfo& init_info)
	{

	}

	VK_RenderPass::VK_RenderPass()
	{

	}

	VK_RenderPass::~VK_RenderPass()
	{

	}

	VkRenderPass& VK_RenderPass::get_render_pass()
	{
		return render_pass;
	}

	VkPipeline& VK_RenderPass::get_pipeline() 
	{
		return pipeline;
	}

}
