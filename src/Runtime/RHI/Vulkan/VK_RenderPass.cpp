#include "VK_RenderPass.h"
#include "VK_Utils.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

	/*
	void VK_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
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

	void VK_RenderPass::initialize_ui_renderbackend(WindowUI* window_ui)
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

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
		_vertexInputInfo = VK_Utils::Vertex_Input_State_Create_Info();
		//connect the pipeline builder vertex input info to the one we get from Vertex
		_vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
		_vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexDescription.attributes.size();

		_vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
		_vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexDescription.bindings.size();


		//make viewport state from our stored viewport and scissor.
			//at the moment we wont support multiple viewports or scissors
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &_viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &_scissor;

		//setup dummy color blending. We arent using transparent objects yet
		//the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;

		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &_colorBlendAttachment;

		//build the actual pipeline
		//we now use all of the info structs we have been writing into into this one to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stageCount = (uint32_t)_shaderStages.size();
		pipelineInfo.pStages = _shaderStages.data();
		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &_depthStencil;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;


		std::vector<VkDynamicState> dynamicStates;
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		dynamicState.pDynamicStates = dynamicStates.data();
		dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();

		pipelineInfo.pDynamicState = &dynamicState;

		//its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}
		else
		{
			VK_GraphicsContext* context=Singleton<DefaultSetting>::get_instance().context.get();
			context->add_on_shutdown_clean_func([=]() {
				vkDestroyPipeline(device,newPipeline,nullptr);
				});
			return newPipeline;
		}
	}

	void PipelineBuilder::clear_vertex_input()
	{
		_vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		_vertexInputInfo.vertexAttributeDescriptionCount = 0;

		_vertexInputInfo.pVertexBindingDescriptions = nullptr;
		_vertexInputInfo.vertexBindingDescriptionCount = 0;
	}

	void PipelineBuilder::set_shaders(VK_Shader* effect)
	{
		_shaderStages.clear();
		effect->fill_stages(_shaderStages);
		_pipelineLayout = effect->get_built_layout();
	}

	VkPipeline ComputePipelineBuilder::build_pipeline(VkDevice device)
	{
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stage = _shaderStage;
		pipelineInfo.layout = _pipelineLayout;


		VkPipeline newPipeline;
		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) 
		{
			return VK_NULL_HANDLE;
		}
		else
		{
			VK_GraphicsContext* context = Singleton<DefaultSetting>::get_instance().context.get();
			context->add_on_shutdown_clean_func([=]() {
				vkDestroyPipeline(device, newPipeline, nullptr);
				});
			return newPipeline;
		}
	}
	*/

VK_RenderPassManager::VK_RenderPassManager(VK_Device* in_device) : device(in_device)
{

}

VK_RenderPassManager::~VK_RenderPassManager()
{
	Destroy();
}

VK_RenderPass* VK_RenderPassManager::GetRenderPass(CONST RenderPassDesc& desc)
{
	RenderPassCacheKey key(desc.attachments.size() - 1, nullptr, desc.attachments[desc.attachments.size() - 1].format, desc.attachments[0].sample_count, false, false);
	for (UInt8 rt = 0; rt < desc.attachments.size() - 1; ++rt)
	{
		key.render_target_formats[rt] = desc.attachments[rt].format;
	}
	auto it = render_pass_cache.find(key);
	if (it != render_pass_cache.end())
	{
		return it->second;
	}
	else
	{
		VK_RenderPass* new_pass = new VK_RenderPass(device, desc);
		render_pass_cache[key] = new_pass;
		return new_pass;
	}
}

VK_RenderPass* VK_RenderPassManager::GetRenderPass(CONST RenderPassCacheKey& key)
{
	auto it = render_pass_cache.find(key);
	if (it != render_pass_cache.end())
	{
		return it->second;
	}
	else
	{
		RenderPassDesc desc;
		desc.attachments.resize(key.num_render_targets + 1);
		desc.attachment_refs.resize(key.num_render_targets + 1);
		for (UInt8 rt = 0; rt < key.num_render_targets; ++rt)
		{
			desc.attachments[rt].format = key.render_target_formats[rt];
			desc.attachments[rt].sample_count = key.sample_count;
			desc.attachments[rt].load_op = ENUM_RENDERPASS_ATTACHMENT_LOAD_OP::Load;
			desc.attachments[rt].store_op = ENUM_RENDERPASS_ATTACHMENT_STORE_OP::Store;
			desc.attachments[rt].initial_state = ENUM_RESOURCE_STATE::RenderTarget;
			desc.attachments[rt].final_state = ENUM_RESOURCE_STATE::RenderTarget;
			desc.attachment_refs[rt].attachment_index = rt;
			desc.attachment_refs[rt].state = ENUM_RESOURCE_STATE::RenderTarget;
		}
		desc.attachments[key.num_render_targets].format = key.depth_stencil_format;
		desc.attachments[key.num_render_targets].sample_count = key.sample_count;
		desc.attachments[key.num_render_targets].load_op = ENUM_RENDERPASS_ATTACHMENT_LOAD_OP::Load;
		desc.attachments[key.num_render_targets].store_op = ENUM_RENDERPASS_ATTACHMENT_STORE_OP::Store;
		ENUM_RESOURCE_STATE depth_state = key.is_read_only_dsv ? ENUM_RESOURCE_STATE::DepthRead : ENUM_RESOURCE_STATE::DepthWrite;
		desc.attachments[key.num_render_targets].initial_state = depth_state;
		desc.attachments[key.num_render_targets].final_state = depth_state;
		desc.attachment_refs[key.num_render_targets].attachment_index = key.num_render_targets;
		desc.attachment_refs[key.num_render_targets].state = depth_state;

		VK_RenderPass* new_pass = new VK_RenderPass(device, desc);
		render_pass_cache[key] = new_pass;
		return new_pass;
	}
}

void VK_RenderPassManager::Destroy()
{
	for (auto it = render_pass_cache.begin(); it != render_pass_cache.end(); ++it)
	{
		it->second->Destroy();
		delete it->second;
	}
	render_pass_cache.clear();
}


VK_RenderPass::VK_RenderPass(VK_Device* in_device, CONST RenderPassDesc& in_desc) : RenderPass(in_desc), device(in_device)
{
	Vector<VkAttachmentDescription> attachment{};
	Vector<VkAttachmentReference> attachment_ref{};
	VkSubpassDescription subpass{};
	VkSubpassDependency dependency{};
	VkRenderPassCreateInfo render_pass_info{};

	attachment.resize(desc.attachments.size());
	attachment_ref.resize(desc.attachment_refs.size());
	
	for(UInt32 i = 0; i < desc.attachments.size(); ++i)
	{
		attachment[i].format = VK_Utils::Translate_Texture_Format_To_Vulkan(desc.attachments[i].format);
		attachment[i].samples = VK_Utils::Translate_Texture_SampleCount_To_Vulkan( desc.attachments[i].sample_count);
		attachment[i].loadOp = VK_Utils::Translate_AttachmentLoad_To_Vulkan(desc.attachments[i].load_op);
		attachment[i].storeOp = VK_Utils::Translate_AttachmentStore_To_Vulkan(desc.attachments[i].store_op);
		attachment[i].stencilLoadOp = VK_Utils::Translate_AttachmentLoad_To_Vulkan(desc.attachments[i].stencil_load_op);
		attachment[i].stencilStoreOp = VK_Utils::Translate_AttachmentStore_To_Vulkan(desc.attachments[i].stencil_store_op);
		attachment[i].initialLayout = VK_Utils::Translate_ReourceState_To_VulkanImageLayout(desc.attachments[i].initial_state,true);
		attachment[i].finalLayout = VK_Utils::Translate_ReourceState_To_VulkanImageLayout(desc.attachments[i].final_state, true);
	}

	for(UInt32 i = 0; i < desc.attachment_refs.size(); ++i)
	{
		attachment_ref[i].attachment = desc.attachment_refs[i].attachment_index;
		attachment_ref[i].layout = VK_Utils::Translate_ReourceState_To_VulkanImageLayout(desc.attachment_refs[i].state, true);
	}

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = attachment_ref.size()-1;	
	subpass.pColorAttachments = attachment_ref.data();
	subpass.pDepthStencilAttachment = &attachment_ref[attachment_ref.size()-1];
	/*
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	*/
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = attachment.size();
	render_pass_info.pAttachments = attachment.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 0;
	render_pass_info.pDependencies = nullptr;
	render_pass_info.pNext = nullptr;
	CHECK_WITH_LOG(vkCreateRenderPass(device->GetDevice(), &render_pass_info, nullptr, &render_pass) != VK_SUCCESS, "RHI Error:Failed to create render pass");

}

VkRenderPass VK_RenderPass::GetRenderPass() CONST
{
	return render_pass;
}

VK_RenderPass::~VK_RenderPass()
{
	Destroy();
}

void VK_RenderPass::Destroy()
{
	if (render_pass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device->GetDevice(), render_pass, nullptr);
		render_pass = VK_NULL_HANDLE;
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
