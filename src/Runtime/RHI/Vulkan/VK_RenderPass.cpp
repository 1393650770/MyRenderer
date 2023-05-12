#include "VK_RenderPass.h"
#include "VK_Viewport.h"
#include "../../UI/Window_UI.h"
#include "VK_Utils.h"
#include "VK_Shader.h"
#include "../../Render/DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "VK_GraphicsContext.h"

namespace MXRender
{


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

}
