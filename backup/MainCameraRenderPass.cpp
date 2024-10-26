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
#include "../../RHI/UniformBuffer.h"
#include "../../RHI/Vulkan/VK_Utils.h"
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../RHI/Vulkan/VK_Texture.h"
#include <array>
#include "../../Logic/GameObjectManager.h"
#include "../TextureManager.h"
namespace MXRender
{

	void MainCamera_RenderPass::setup_renderpass()
	{
		

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = color_image_format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = cur_context.lock()->get_depth_image_format();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(cur_context.lock()->device->device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}


		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = color_image_format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = cur_context.lock()->get_depth_image_format();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


			VkAttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(cur_context.lock()-> device->device, &renderPassInfo, nullptr, &clear_pass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

	}

	void MainCamera_RenderPass::setup_descriptorset_layout()
	{
		std::shared_ptr<VK_Device> device = cur_context.lock()->device;

		descriptorset_layout=std::make_shared<VK_DescriptorSetLayout>(device,5);

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding cubetextureLayoutBinding{};
		cubetextureLayoutBinding.binding = 1;
		cubetextureLayoutBinding.descriptorCount = 1;
		cubetextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cubetextureLayoutBinding.pImmutableSamplers = nullptr;
		cubetextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		descriptorset_layout->add_bindingdescriptor(0, uboLayoutBinding);

		descriptorset_layout->add_bindingdescriptor(1, cubetextureLayoutBinding);
		descriptorset_layout->compile();
	}

	void MainCamera_RenderPass::setup_pipelines()
	{
		std::shared_ptr<VK_Device> device= cur_context.lock()->device;

		VK_Shader cur_shader(device, "Shader/skybox_vert.spv","Shader/skybox_frag.spv");

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
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;	
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;


		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	


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
		pipelineLayoutInfo.setLayoutCount = 1;
		//pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pSetLayouts = &descriptorset_layout->get_descriptorset_layout();

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
		pipelineInfo.pDepthStencilState = &depthStencil;
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
	
		VK_Utils::Create_Image(cur_context.lock()->device->gpu,
			cur_context.lock()->device->device,
			cur_context.lock()->get_swapchain_extent().width,
			cur_context.lock()->get_swapchain_extent().height,
			color_image_format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			color_image,
			color_image_memory,
			0,
			1,
			1);
		color_imageview = VK_Utils::Create_ImageView(cur_context.lock()->device->device,
			color_image,
			color_image_format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_VIEW_TYPE_2D,
			1,
			1);
		color_image_sampler = VK_Utils::Create_Linear_Sampler(cur_context.lock()->device->gpu, cur_context.lock()->device->device);
		{
			
			std::array<VkImageView, 2> attachments = {
			color_imageview,
			cur_context.lock()->get_depth_image_view()
			};
			VkFramebufferCreateInfo framebuffer_create_info{};
			framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_create_info.renderPass = render_pass;
			framebuffer_create_info.attachmentCount = 2;
			framebuffer_create_info.pAttachments = attachments.data();
			framebuffer_create_info.width = cur_context.lock()->get_swapchain_extent().width;
			framebuffer_create_info.height = cur_context.lock()->get_swapchain_extent().height;
			framebuffer_create_info.layers = 1;
			if (vkCreateFramebuffer(
				cur_context.lock()->device->device, &framebuffer_create_info, nullptr, &framebuffer) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("create main camera framebuffer");
			}
		}
	}

	void MainCamera_RenderPass::setup_uniformbuffer()
	{
		VkDeviceSize bufferSize = sizeof(MVP_Struct);

		uniform_buffers.resize(1);
		uniform_buffers_memory.resize(1);

		for (size_t i = 0; i < 1; i++) {
			VK_Utils::Create_VKBuffer( cur_context.lock()->device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers[i], uniform_buffers_memory[i]);
		}
	}

	void MainCamera_RenderPass::setup_descriptorpool()
	{
		std::vector<VkDescriptorPoolSize> poolSize(2);
		poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize[0].descriptorCount = static_cast<uint32_t>(cur_context.lock()->get_max_frame_num());
		poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize[1].descriptorCount = static_cast<uint32_t>(cur_context.lock()->get_max_frame_num());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSize.size();
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = static_cast<uint32_t>(3);

		if (vkCreateDescriptorPool(cur_context.lock()->device->device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void MainCamera_RenderPass::setup_descriptorsets()
	{
		
		std::vector<std::string> cubemap_path{

			"Resource/Texture/Skybox/right.jpg",
			"Resource/Texture/Skybox/left.jpg",
			"Resource/Texture/Skybox/top.jpg",
			"Resource/Texture/Skybox/bottom.jpg",
			"Resource/Texture/Skybox/front.jpg",
			"Resource/Texture/Skybox/back.jpg",

		};
		std::string cubemap_path2 = "Resource/Texture/Skybox/kyoto_lod.dds";
		std::string cubemap_path3 = "Resource/Texture/Skybox/bolonga_lod.dds";


		VK_Texture* cubemap_texture= Singleton<DefaultSetting>::get_instance().texture_manager->get_or_create_texture("skybox",ENUM_TEXTURE_TYPE::ENUM_TYPE_CUBE_MAP, cubemap_path3);
		if (cubemap_texture==nullptr)
		{
			return;
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptor_pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorset_layout->get_descriptorset_layout();
		descriptor_sets.resize(1);
		if (vkAllocateDescriptorSets(cur_context.lock()->device->device, &allocInfo, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < 1; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniform_buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(MVP_Struct);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = cubemap_texture->texture_image_view;
			imageInfo.sampler = cubemap_texture->texture_sampler;

			std::vector< VkWriteDescriptorSet> descriptorWrite(2);
			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = descriptor_sets[i];
			descriptorWrite[0].dstBinding = 0;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite[0].descriptorCount = 1;
			descriptorWrite[0].pBufferInfo = &bufferInfo;

			descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[1].dstSet = descriptor_sets[i];
			descriptorWrite[1].dstBinding = 1;
			descriptorWrite[1].dstArrayElement = 0;
			descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite[1].descriptorCount = 1;
			descriptorWrite[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(cur_context.lock()->device->device, descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
		}
		
	}

	void MainCamera_RenderPass::update_uniformbuffer()
	{


		MVP_Struct ubo{};
		ubo.model = glm::mat4(1.0f);
	/*	ubo.view = glm::mat4(glm::mat3(glm::lookAt(glm::vec3(0.0f, 0.0f, 30.0f), glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f))));*/
		ubo.view = glm::mat4(glm::mat3(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_view_mat()));
		ubo.proj = Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_projection_mat();
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(cur_context.lock()->device->device, uniform_buffers_memory[0], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(cur_context.lock()->device->device, uniform_buffers_memory[0]);
	}	

	void MainCamera_RenderPass::destroy_framebuffer()
	{
		vkDestroyImageView(cur_context.lock()->device->device, color_imageview, nullptr);

		vkDestroyImage(cur_context.lock()->device->device, color_image, nullptr);
		vkFreeMemory(cur_context.lock()->device->device, color_image_memory, nullptr);
		vkDestroySampler(cur_context.lock()->device->device, color_image_sampler, nullptr);
		vkDestroyFramebuffer(cur_context.lock()->device->device, framebuffer, nullptr);

	}



	void MainCamera_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
	{
		pass_info = init_info;
		VKPassCommonInfo* vk_info= static_cast<VKPassCommonInfo*>(other_info);
		cur_context=vk_info->context;
		setup_renderpass();
		setup_descriptorset_layout();
		setup_pipelines();
		setup_framebuffer();
		setup_uniformbuffer();
		setup_descriptorpool();
		setup_descriptorsets();
		cur_context.lock()->add_on_swapchain_recreate_func(std::bind(&MainCamera_RenderPass::setup_framebuffer, this));
		cur_context.lock()->add_on_swapchain_clean_func(std::bind(&MainCamera_RenderPass::destroy_framebuffer, this));

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = cur_context.lock()->get_swapchain_extent().width;
		viewport.height = cur_context.lock()->get_swapchain_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
	}

	void MainCamera_RenderPass::destroy()
	{
		destroy_framebuffer();
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




	void MainCamera_RenderPass::draw(GraphicsContext* context, RenderScene* render_scene)
	{
		
		update_uniformbuffer();

		VK_GraphicsContext* vk_context=nullptr;
		vk_context= dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context==nullptr)
		{	
			return;
		}

		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);


		viewport.width = vk_context->get_swapchain_extent().width;
		viewport.height = vk_context->get_swapchain_extent().height;

		vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_context->get_swapchain_extent();

		vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

		vkCmdBindDescriptorSets(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[0], 0, nullptr);

		vkCmdDraw(vk_context->get_cur_command_buffer(), 36, 1, 0, 0);


	}



	void MainCamera_RenderPass::begin_pass(GraphicsContext* context)
	{
		VK_GraphicsContext* vk_context = nullptr;
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context == nullptr)
		{
			return;
		}
		uint32_t image_index = -1;
		image_index = vk_context->get_current_swapchain_image_index();
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vk_context->get_swapchain_extent();

		std::vector< VkClearValue> clearColor(2);
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = 0;
		renderPassInfo.pClearValues = nullptr;//clearColor.data();

		//VK_Utils::Transition_ImageLayout(cur_context,cur_context.lock()->get_cur_swapchain_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		//VK_Utils::Transition_ImageLayout(cur_context, cur_context.lock()->get_depth_image(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT);

		//VK_Utils::ClearImageColor(cur_context, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cur_context.lock()->get_cur_swapchain_image(), VK_IMAGE_ASPECT_COLOR_BIT);

		//VK_Utils::Transition_ImageLayout(cur_context, cur_context.lock()->get_cur_swapchain_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_Utils::Immediate_Submit(vk_context,[&](VkCommandBuffer cmd)
		{
				uint32_t image_index = -1;
				image_index = vk_context->get_current_swapchain_image_index();
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = clear_pass;
				renderPassInfo.framebuffer = framebuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = vk_context->get_swapchain_extent();

				std::vector< VkClearValue> clearColor(2);
				clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
				clearColor[1].depthStencil = { 1.0f, 0 };
				renderPassInfo.clearValueCount = 2;
				renderPassInfo.pClearValues = clearColor.data();

				vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdEndRenderPass(cmd);
		});

		vkCmdBeginRenderPass(vk_context->get_cur_command_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vk_context->inheritance_info_map[render_pass].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		vk_context->inheritance_info_map[render_pass].framebuffer = framebuffer;
		vk_context->inheritance_info_map[render_pass].renderPass = render_pass;
		
		vk_context->renderpass_begin_info_map[render_pass]= renderPassInfo;
	}

	void MainCamera_RenderPass::end_pass(GraphicsContext* context)
	{
		VK_GraphicsContext* vk_context = nullptr;
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context == nullptr)
		{
			return;
		}
		vkCmdEndRenderPass(vk_context->get_cur_command_buffer());

		/*VK_Utils::Transition_ImageLayout(cur_context, vk_context->get_cur_command_buffer(), cur_context.lock()->get_cur_swapchain_image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT);*/
	}

	MainCamera_RenderPass::MainCamera_RenderPass()
	{

	}

	MainCamera_RenderPass::~MainCamera_RenderPass()
	{
		vkDestroyPipeline(cur_context.lock()->device->device, pipeline, nullptr);
		vkDestroyPipelineLayout(cur_context.lock()->device->device, pipeline_layout, nullptr);
		vkDestroyRenderPass(cur_context.lock()->device->device, render_pass, nullptr);
		vkDestroyRenderPass(cur_context.lock()->device->device, clear_pass, nullptr);

		for (size_t i = 0; i < uniform_buffers.size(); i++) {
			vkDestroyBuffer(cur_context.lock()->device->device, uniform_buffers[i], nullptr);
			vkFreeMemory(cur_context.lock()->device->device, uniform_buffers_memory[i], nullptr);
		}
		//vkFreeDescriptorSets(cur_context.lock()->device->device, descriptor_pool, descriptor_sets.size(), descriptor_sets.data());
		vkDestroyDescriptorPool(cur_context.lock()->device->device, descriptor_pool, nullptr);

	}

	VkRenderPass& MainCamera_RenderPass::get_render_pass() 
	{
		return render_pass;
	}


	MXRender::MainCamera_RenderPass::MainCamera_RenderPass(const PassInfo& init_info)
    {
        pass_info = init_info;
    }
}
