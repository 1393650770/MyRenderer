#include "CopyPass.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan_core.h"
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
#include "../../UI/Window_UI.h"
#include <chrono>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"



namespace MXRender
{ 

	void MXRender::Copy_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
	{
		VKPassCommonInfo* vk_info = static_cast<VKPassCommonInfo*>(other_info);
		cur_context = vk_info->context;
		pass_info = init_info;
		
		setup_pipeline();
		setup_renderpass();
		setup_framebuffer();
		cur_context.lock()->add_on_swapchain_recreate_func(std::bind(&Copy_RenderPass::setup_framebuffer, this));
		cur_context.lock()->add_on_swapchain_clean_func(std::bind(&Copy_RenderPass::destroy_framebuffer, this));
	}

	void Copy_RenderPass::setup_renderpass()
	{

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

			if (vkCreateRenderPass(cur_context.lock()->device->device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

		Singleton<DefaultSetting>::get_instance().material_system->psos["copy_one_texture_pso"] =Singleton<DefaultSetting>::get_instance().material_system->build_pso(render_pass,copy_pipeline_builder, Singleton<DefaultSetting>::get_instance().material_system->shaders["copy_one_texture"]);
		pso= Singleton<DefaultSetting>::get_instance().material_system->psos["copy_one_texture_pso"];
	}

	void Copy_RenderPass::setup_pipeline()
	{
		{

			copy_pipeline_builder.clear_vertex_input();

			copy_pipeline_builder._inputAssembly = VK_Utils::Input_Assembly_Create_Info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);


			copy_pipeline_builder._rasterizer = VK_Utils::Rasterization_State_Create_Info(VK_POLYGON_MODE_FILL);
			copy_pipeline_builder._rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//BACK_BIT;
			copy_pipeline_builder._rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			copy_pipeline_builder._multisampling = VK_Utils::Multisampling_State_Create_Info();

			copy_pipeline_builder._colorBlendAttachment = VK_Utils::Color_Blend_Attachment_State();

			//default depthtesting
			copy_pipeline_builder._depthStencil = VK_Utils::Depth_Stencil_Create_Info(false, false, VK_COMPARE_OP_ALWAYS);
		}
	}

	void Copy_RenderPass::setup_framebuffer()
	{
		unsigned int swap_chain_images_num = cur_context.lock()->swapchain_imageviews.size();
		swapchain_framebuffers.resize(swap_chain_images_num);
		cur_context.lock()->swapchain_framebuffers.resize(swap_chain_images_num);
		for (int i = 0; i < swap_chain_images_num; i++)
		{
			std::array<VkImageView, 2> attachments = {
					cur_context.lock()->swapchain_imageviews[i],
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
				cur_context.lock()->device->device, &framebuffer_create_info, nullptr, &swapchain_framebuffers[i]) !=
				VK_SUCCESS)
			{
				throw std::runtime_error("create main camera framebuffer");
			}
			cur_context.lock()->swapchain_framebuffers[i]=&swapchain_framebuffers[i];
		}


	}

	void Copy_RenderPass::destroy_framebuffer()
	{
		for (size_t i = 0; i < swapchain_framebuffers.size(); i++) {
			vkDestroyFramebuffer(cur_context.lock()->device->device, swapchain_framebuffers[i], nullptr);
			cur_context.lock()->swapchain_framebuffers[i] = nullptr;
		}
	}

	Copy_RenderPass::~Copy_RenderPass()
	{

	}

	void Copy_RenderPass::begin_pass(GraphicsContext* context)
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
		renderPassInfo.framebuffer = *(vk_context->swapchain_framebuffers[image_index]);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vk_context->get_swapchain_extent();

		std::vector< VkClearValue> clearColor(2);
		clearColor[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearColor[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = clearColor.size();
		renderPassInfo.pClearValues = clearColor.data();
		vkCmdBeginRenderPass(vk_context->get_cur_command_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	}

	void Copy_RenderPass::end_pass(GraphicsContext* context)
	{
		VK_GraphicsContext* vk_context = nullptr;
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context == nullptr)
		{
			return;
		}
		vkCmdEndRenderPass(vk_context->get_cur_command_buffer());
	}

	void Copy_RenderPass::draw(GraphicsContext* context, RenderScene* render_scene/*=nullptr*/)
	{
		VK_GraphicsContext* vk_context = nullptr;
		vk_context = dynamic_cast<VK_GraphicsContext*>(context);
		if (vk_context == nullptr)
		{
			return;
		}
		vkCmdSetViewport(vk_context->get_cur_command_buffer(), 0, 1, &vk_context->viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = vk_context->get_swapchain_extent();

		vkCmdSetScissor(vk_context->get_cur_command_buffer(), 0, 1, &scissor);

		vkCmdSetDepthBias(vk_context->get_cur_command_buffer(), 0, 0, 0);

		vkCmdBindPipeline(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso->pipeline);



		vkCmdBindDescriptorSets(vk_context->get_cur_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pso->pipeline_layout, 0, 1, &blitSet, 0, nullptr);

		vkCmdDraw(vk_context->get_cur_command_buffer(), 3, 1, 0, 0);

	}

	void Copy_RenderPass::build_input_set(VkDescriptorImageInfo image_info)
	{
		DescriptorBuilder::begin(Singleton<DefaultSetting>::get_instance().material_system->get_descriptorlayout_cache(), Singleton<DefaultSetting>::get_instance().material_system->get_descript_temp_pool())
			.bind_image(0, &image_info, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(blitSet);
	}

}
