#include "MainCameraRenderPass.h"
#include "../../RHI/Vulkan/VK_GraphicsContext.h"
#include "../../RHI/Vulkan/VK_Device.h"
#include "../../RHI/Vulkan/VK_Shader.h"
#include "../../RHI/Vulkan/VK_VertexArray.h"
#include "../../RHI/Vulkan/VK_DescriptorSets.h"
#include "../../RHI/Vulkan/VK_Viewport.h"
#include "../../RHI/Vulkan/VK_SwapChain.h"
#include "../DefaultSetting.h"
#include "../../Utils/Singleton.h"
#include "vulkan/vulkan_core.h"
#include "../../RHI/RenderEnum.h"
namespace MXRender
{

	void MainCamera_RenderPass::setup_renderpass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = cur_context.lock()->get_swapchain_image_format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(cur_context.lock()->device->device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void MainCamera_RenderPass::setup_descriptorset_layout()
	{
		std::shared_ptr<VK_Device> device = cur_context.lock()->device;

		layout=std::make_shared<VK_DescriptorSetLayout>(device,1);

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		layout->add_bindingdescriptor(0, uboLayoutBinding);

		layout->compile();
	}

	void MainCamera_RenderPass::setup_pipelines()
	{
		std::shared_ptr<VK_Device> device= cur_context.lock()->device;

		VK_Shader cur_shader(device, VK_SHADER_STAGE_VERTEX_BIT,"Shader/vert.spv","Shader/frag.spv");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = cur_shader.shader_modules[ENUM_SHADER_STAGE::Shader_Vertex];
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = cur_shader.shader_modules[ENUM_SHADER_STAGE::Shader_Pixel];
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(cur_context.lock()->device->device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipeline_layout;
		pipelineInfo.renderPass = render_pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(cur_context.lock()->device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

	void MainCamera_RenderPass::setup_framebuffer()
	{
		unsigned int swap_chain_images_num  = cur_context.lock()->swapchain_imageviews.size();
		swapchain_framebuffers.resize(swap_chain_images_num);

		for (int i = 0; i < swap_chain_images_num; i++)
		{
			VkFramebufferCreateInfo framebuffer_create_info{};
			framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_create_info.renderPass = render_pass;
			framebuffer_create_info.attachmentCount =1;
			framebuffer_create_info.pAttachments = &cur_context.lock()->swapchain_imageviews[i];
			framebuffer_create_info.width = cur_context.lock()->get_swapchain_extent().width;
			framebuffer_create_info.height = cur_context.lock()->get_swapchain_extent().height;
			framebuffer_create_info.layers = 1;
			
			if (vkCreateFramebuffer(
				cur_context.lock()->device->device, &framebuffer_create_info, nullptr, &swapchain_framebuffers[i]) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("create main camera framebuffer");
			}
		}
	}

	void MainCamera_RenderPass::initialize(const PassInfo& init_info, std::shared_ptr<GraphicsContext> context) 
	{
	}

	void MainCamera_RenderPass::initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context, std::weak_ptr<VK_Viewport> viewport)
	{
		pass_info = init_info;
		this->cur_context = context;
		setup_renderpass();
		setup_descriptorset_layout();
		setup_pipelines();
		setup_framebuffer();
	}

	void MainCamera_RenderPass::initialize(const PassInfo& init_info, std::shared_ptr<VK_GraphicsContext> context)
	{
		pass_info = init_info;
		this->cur_context = context;
		setup_renderpass();
		setup_descriptorset_layout();
		setup_pipelines();
		setup_framebuffer();
	}

	void MainCamera_RenderPass::post_initialize()
    {
    }

    void MainCamera_RenderPass::set_commonInfo(const PassInfo& init_info)
    {
    }

    void MainCamera_RenderPass::prepare_pass_data(const GraphicsContext& context)
    {
    }

    void MainCamera_RenderPass::initialize_ui_renderbackend()
    {
    }


	void MainCamera_RenderPass::draw(GraphicsContext* context)
	{
		VK_GraphicsContext* vk_context=nullptr;
		vk_context= dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context==nullptr)
		{	
			return;
		}
		uint32_t image_index=-1;
		image_index=vk_context->get_current_swapchain_image_index();
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;	
		renderPassInfo.framebuffer = swapchain_framebuffers[image_index];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vk_context->get_swapchain_extent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(vk_context->get_cur_command_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport vk_viewport{};
		
		vk_viewport.x = 0.0f;
		vk_viewport.y = 0.0f;
		vk_viewport.width = (float)vk_context->get_swapchain_extent().width;
		vk_viewport.height = (float)vk_context->get_swapchain_extent().height;
		vk_viewport.minDepth = 0.0f;
		vk_viewport.maxDepth = 1.0f;

		vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &vk_viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_context->get_swapchain_extent();

		vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

		vkCmdDraw(vk_context->get_cur_command_buffer(), 3, 1, 0, 0);

		vkCmdEndRenderPass(vk_context->get_cur_command_buffer());
	}



    MainCamera_RenderPass::MainCamera_RenderPass()
	{

	}

	MainCamera_RenderPass::~MainCamera_RenderPass()
	{
		vkDestroyPipeline(cur_context.lock()->device->device, pipeline, nullptr);
		vkDestroyPipelineLayout(cur_context.lock()->device->device, pipeline_layout, nullptr);

		for (auto framebuffer : swapchain_framebuffers) {
			vkDestroyFramebuffer(cur_context.lock()->device->device, framebuffer, nullptr);
		}
	}

	VkRenderPass& MainCamera_RenderPass::get_render_pass() 
	{
		return render_pass;
	}

	std::vector<VkFramebuffer>& MainCamera_RenderPass::get_swapchain_framebuffers()
	{
		return swapchain_framebuffers;
	}

	MXRender::MainCamera_RenderPass::MainCamera_RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
