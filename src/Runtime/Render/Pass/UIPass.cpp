#include "UIPass.h"
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
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"







void MXRender::UI_RenderPass::upload_fonts()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = cur_context.lock()->command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = {};
	if (VK_SUCCESS != vkAllocateCommandBuffers(cur_context.lock()->device->device, &allocInfo, &commandBuffer))
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo))
	{
		throw std::runtime_error("Could not create one-time command buffer!");
	}

	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer))
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(cur_context.lock()->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(cur_context.lock()->graphicsQueue);

	vkFreeCommandBuffers(cur_context.lock()->device->device, cur_context.lock()->command_pool, 1, &commandBuffer);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void MXRender::UI_RenderPass::initialize(const PassInfo& init_info, PassOtherInfo* other_info)
{
	VKPassCommonInfo* vk_info = static_cast<VKPassCommonInfo*>(other_info);
	cur_context = vk_info->context;
	pass_info = init_info;
	render_pass = vk_info->render_pass;
}

void MXRender::UI_RenderPass::initialize_ui_renderbackend(WindowUI* window_ui)
{
	this->window_ui = window_ui;

	ImGui_ImplGlfw_InitForVulkan(cur_context.lock()->get_window(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = cur_context.lock()->get_instance();
	init_info.PhysicalDevice = cur_context.lock()->device->gpu;
	init_info.Device = cur_context.lock()->device->device;
	init_info.QueueFamily = cur_context.lock()->get_queuefamily().graphicsFamily.value();
	init_info.Queue = cur_context.lock()->graphicsQueue;
	init_info.DescriptorPool = cur_context.lock()->descriptor_pool;
	//init_info.Subpass = _main_camera_subpass_ui;


	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	ImGui_ImplVulkan_Init(&init_info, render_pass);

	upload_fonts();
}

void MXRender::UI_RenderPass::draw(GraphicsContext* context)
{
	if (window_ui)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		window_ui->pre_render();

		//if (cur_context.lock()->enableValidationLayers)
		//{
		//	VkDebugUtilsLabelEXT label_info = {
		//		VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, nullptr, "ImGUI", {1.0f, 1.0f, 1.0f, 1.0f} };
		//	vkCmdBeginDebugUtilsLabelEXT(cur_context.lock()->get_cur_command_buffer(), &label_info);
		//}

		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cur_context.lock()->get_cur_command_buffer());

		//if (cur_context.lock()->enableValidationLayers)
		//{
		//	vkCmdEndDebugUtilsLabelEXT(cur_context.lock()->get_cur_command_buffer());
		//	
		//}
	}
}
