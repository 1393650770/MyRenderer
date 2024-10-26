#include "VK_CommandBuffer.h"
#include "VK_Device.h"
#include "VK_Fence.h"
#include "VK_Utils.h"
#include "VK_Define.h"
#include "VK_PipelineState.h"
#include "VK_RenderPass.h"
#include "VK_Texture.h"
#include "VK_FrameBuffer.h"
#include "Core/ConstDefine.h"
#include "VK_Shader.h"
#define  GLFW_INCLUDE_VULKAN
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "GLFW/glfw3.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


VK_CommandBufferPool::VK_CommandBufferPool(VK_Device* in_device, VK_CommandBufferManager& in_cmd_buffer_manager):device(in_device),cmd_buffer_manager(in_cmd_buffer_manager)
{

}

VK_CommandBufferPool::~VK_CommandBufferPool()
{
	for (UInt32 index = 0; index < cmd_buffers.size(); ++index)
	{
		delete cmd_buffers[index];
	}
	for (UInt32 index = 0; index < free_cmd_buffers.size(); ++index)
	{
		delete free_cmd_buffers[index];
	}
	vkDestroyCommandPool(device->GetDevice(), command_pool, VULKAN_CPU_ALLOCATOR);
	command_pool=VK_NULL_HANDLE;
}

VK_CommandBuffer* VK_CommandBufferPool::GetOrCreateCommandBuffer(Bool is_upload_only)
{
	for (Int index = free_cmd_buffers.size()-1; index>=0;--index)
	{
		VK_CommandBuffer* cmd_buffer = free_cmd_buffers[index];
		if (cmd_buffer->is_upload_only == is_upload_only)
		{
			free_cmd_buffers.erase(free_cmd_buffers.begin() + index);
			cmd_buffer->Allocate();
			cmd_buffers.push_back(cmd_buffer);
			return cmd_buffer;
		}
	}
	VK_CommandBuffer* cmd_buffer = new VK_CommandBuffer(device, this, is_upload_only);
	cmd_buffers.push_back(cmd_buffer);
	return cmd_buffer;
}

void VK_CommandBufferPool::Init(UInt32 queue_family_index)
{
	VkCommandPoolCreateInfo command_pool_create_info;
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.queueFamilyIndex = queue_family_index;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.pNext = nullptr;
	CHECK_WITH_LOG(vkCreateCommandPool(device->GetDevice(), &command_pool_create_info, VULKAN_CPU_ALLOCATOR, &command_pool) != VK_SUCCESS, "RHI Error: Create CommandPool Error");
}

void VK_CommandBufferPool::FreeUnusedCommandBuffers(VK_Queue* queue)
{
	for (Int index = cmd_buffers.size() - 1; index >= 0; --index)
	{
		VK_CommandBuffer* cmd_buffer = cmd_buffers[index];
		if ( cmd_buffer->GetFence()->GetIsSignaled())
		{
			cmd_buffer->Free();
			free_cmd_buffers.push_back(cmd_buffer);
			cmd_buffers.erase(cmd_buffers.begin() + index);
			return;
		}
	}
}

void VK_CommandBufferPool::FreeUnusedCommandBuffer(VK_CommandBuffer* target)
{
	for (Int index = cmd_buffers.size() - 1; index >= 0; --index)
	{
		VK_CommandBuffer* cmd_buffer = cmd_buffers[index];
		if (target == cmd_buffer && cmd_buffer->GetFence()->GetIsSignaled())
		{
			cmd_buffer->Free();
			free_cmd_buffers.push_back(cmd_buffer);
			cmd_buffers.erase(cmd_buffers.begin() + index);
			return;
		}
	}
}

VkCommandPool VK_CommandBufferPool::GetPool() CONST
{
	return command_pool;
}


VK_CommandBuffer::~VK_CommandBuffer()
{
	if (false)
	{
		// Wait 33ms
		UInt64 wait_seconds = 33 * 1000 * 1000LL;
		device->GetFenceManager()->WaitForFence(fence, wait_seconds);
	}
	if (fence)
	{
		device->GetFenceManager()->FreeFence(fence);
		fence=nullptr;
	}
	if(command_buffer!=VK_NULL_HANDLE)
	{ 
		Free();
		command_buffer=VK_NULL_HANDLE;
	}
}

VkCommandBuffer VK_CommandBuffer::GetCommandBuffer() CONST
{
	return command_buffer;
}

VK_Fence* VK_CommandBuffer::GetFence() CONST
{
	return fence;
}

void VK_CommandBuffer::Allocate()
{
	VkCommandBufferAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandBufferCount =1;
	allocate_info.level= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandPool=owner_pool->GetPool();
	CHECK_WITH_LOG(vkAllocateCommandBuffers(device->GetDevice(), &allocate_info, &command_buffer) != VK_SUCCESS,
		"RHI Error: failed to allocate command buffers!");
}

void VK_CommandBuffer::Free()
{
	CHECK_WITH_LOG(command_buffer==VK_NULL_HANDLE,"RHI Error: command buffer is nullptr when freeing commandbuffer!");
	vkFreeCommandBuffers(device->GetDevice(), owner_pool->GetPool(), 1, &command_buffer);
	command_buffer=VK_NULL_HANDLE;
	
}

void VK_CommandBuffer::ClearTexture(Texture* texture, Vector<float> clear_value /*= Vector<float>(4, 0.0f)*/)
{
	static constexpr CONST UInt32 invalid_attachment_index = ~UInt32{ 0 };

	bool clear_depth_attachment = false,clear_depth_image=false;
	UInt32 attachment_index = invalid_attachment_index;
	for (UInt32 rt = 0; rt < state_cache.render_targets.size(); ++rt)
	{
		if (state_cache.render_targets[rt] == texture)
		{
			attachment_index = rt;
			break;
		}
	}
	if (attachment_index == invalid_attachment_index)
	{
		if (state_cache.depth_stencil == texture)
		{
			clear_depth_attachment = true;
		}
		else
		{
			if ((UInt32)(texture->GetTextureDesc().usage & ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT) == 1 ||
				(UInt32)(texture->GetTextureDesc().usage & ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT_READ_ONLY) == 1 ||
				(UInt32)(texture->GetTextureDesc().usage & ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_DEPTH_ATTACHMENT_WRITE_ONLY) == 1
				)
			{
				clear_depth_image = true;
			}
		}
	}
	if (attachment_index != invalid_attachment_index)
	{
		VkClearAttachment clear_attachment{};
		clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		clear_attachment.colorAttachment = attachment_index;
		clear_attachment.clearValue = { clear_value[0], clear_value[1], clear_value[2], clear_value[3] };
		VkClearRect clear_rect{};
		clear_rect.baseArrayLayer = 0;
		clear_rect.layerCount = texture->GetTextureDesc().layer_count;
		clear_rect.rect.offset = { 0, 0 };
		clear_rect.rect.extent = { texture->GetTextureDesc().width, texture->GetTextureDesc().height };
		ClearAttachment(clear_attachment, clear_rect);
	}
	else if (clear_depth_attachment)
	{
		VkClearAttachment clear_attachment{};
		clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		clear_attachment.clearValue.depthStencil.depth = clear_value[0];
		clear_attachment.clearValue.depthStencil.stencil = static_cast<UInt32>(clear_value[1]);
		VkClearRect clear_rect{};
		clear_rect.baseArrayLayer = 0;
		clear_rect.layerCount = texture->GetTextureDesc().layer_count;
		clear_rect.rect.offset = { 0, 0 };
		clear_rect.rect.extent = { texture->GetTextureDesc().width, texture->GetTextureDesc().height };
		ClearAttachment(clear_attachment, clear_rect);
	}
	else if (clear_depth_image)
	{
		TransitionTextureState(texture, ENUM_RESOURCE_STATE::CopyDest);
		VkImageSubresourceRange subresource_range{};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		subresource_range.baseArrayLayer = 0;
		subresource_range.layerCount = texture->GetTextureDesc().layer_count;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = texture->GetTextureDesc().mip_level;
		VkClearDepthStencilValue clear_value_depth_stencil{};
		clear_value_depth_stencil.depth = clear_value[0];
		clear_value_depth_stencil.stencil = static_cast<UInt32>(clear_value[1]);
		ClearDepthStencilImage(((VK_Texture*)texture)->GetImage(), clear_value_depth_stencil, subresource_range);
	}
	else
	{
		TransitionTextureState(texture, ENUM_RESOURCE_STATE::CopyDest);
		VkImageSubresourceRange subresource_range{};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.baseArrayLayer = 0;
		subresource_range.layerCount = texture->GetTextureDesc().layer_count;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = texture->GetTextureDesc().mip_level;
		VkClearColorValue clear_color_value{};
		for (UInt32 i = 0; i < 4; ++i)
		{
			clear_color_value.float32[i] = clear_value[i];
		}
		ClearColorImage(((VK_Texture*)texture)->GetImage(), clear_color_value, subresource_range);

	}
}

void VK_CommandBuffer::TransitionTextureState(Texture* texture, CONST ENUM_RESOURCE_STATE& required_state)
{
	if (texture->GetTextureDesc().resource_state != required_state)
	{
		ENUM_RESOURCE_STATE old_state = texture->GetTextureDesc().resource_state;
		VkImageSubresourceRange subres_range;
		subres_range.aspectMask = 0;
		subres_range.baseArrayLayer = 0;
		subres_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
		subres_range.baseMipLevel = 0;
		subres_range.levelCount = VK_REMAINING_MIP_LEVELS;
		if (subres_range.aspectMask == 0)
		{
			CONST auto& tex_desc = texture->GetTextureDesc();
			CONST auto& fmt_attribs = texture->GetTextureFormatAttribs(tex_desc.format);
			if (fmt_attribs.component_format == ENUM_TEXTURE_COMPONENT_FORMAT::Depth)
				subres_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			else if (fmt_attribs.component_format == ENUM_TEXTURE_COMPONENT_FORMAT::DepthStencil)
			{
				// If image has a depth / stencil format with both depth and stencil components, then the
				// aspectMask member of subresourceRange must include both VK_IMAGE_ASPECT_DEPTH_BIT and
				// VK_IMAGE_ASPECT_STENCIL_BIT 
				subres_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else
				subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		// Always add barrier after writes.
		CONST Bool after_write = VK_Utils::Check_ResourceState_Has_WriteAccess(old_state);

		CONST auto  old_layout = VK_Utils::Translate_ReourceState_To_VulkanImageLayout(old_state,command_state==EState::IsInsideRenderPass);
		CONST auto  new_layout = VK_Utils::Translate_ReourceState_To_VulkanImageLayout(required_state, command_state == EState::IsInsideRenderPass);
		CONST auto  old_stages  =  VK_Utils::Translate_ReourceState_To_VulkanPipelineStage(old_state);
		CONST auto  new_stages  =  VK_Utils::Translate_ReourceState_To_VulkanPipelineStage(required_state);

		if (((old_state & required_state) != required_state) || old_layout != new_layout || after_write)
		{
			VK_Texture* vk_texture = STATIC_CAST( texture, VK_Texture);
			TrainsitionImageLayout(vk_texture->GetImage(), old_layout, new_layout, subres_range, old_stages, new_stages);
			texture->SetResourceState(required_state);
		}
	}

}

void VK_CommandBuffer::BeginUI()
{
	ImGui_ImplVulkan_NewFrame();

	ImGui_ImplGlfw_NewFrame();
	
	ImGui::NewFrame();
}

void VK_CommandBuffer::EndUI()
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),command_buffer );
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void VK_CommandBuffer::TransitionRenderTargets(CONST Vector<Texture*>& render_targets, Texture* depth_stencil)
{
	if (render_targets.empty() && depth_stencil == nullptr)
	{
		return;
	}
	if (!render_targets.empty())
	{
		for (auto& rtv : render_targets)
		{
			TransitionTextureState(rtv, ENUM_RESOURCE_STATE::RenderTarget);
		}
	}
	if (depth_stencil != nullptr)
	{
		TransitionTextureState(depth_stencil, ENUM_RESOURCE_STATE::DepthWrite);
	}
}

VK_CommandBuffer::VK_CommandBuffer(VK_Device* in_device, VK_CommandBufferPool* in_command_buffer_pool, Bool in_is_upload_only):
		device(in_device),
		owner_pool(in_command_buffer_pool),
		is_upload_only(in_is_upload_only)
{
	Allocate();
	fence=device->GetFenceManager()->GetOrCreateFence();
	command_state = EState::ReadyForBegin;
}

UInt64 VK_CommandBuffer::GetFenceSignaledCounter() CONST
{
	return fence_signaled_counter;
}

Bool VK_CommandBuffer::WaitForFence(float time_in_seconds_to_wait)
{
	return device->GetFenceManager()->WaitForFence(fence, time_in_seconds_to_wait);
}

void VK_CommandBuffer::TrainsitionImageLayout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,CONST VkImageSubresourceRange& subresource_range, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask)
{
	//if (state_cache.render_pass != VK_NULL_HANDLE)
	//{
	//	EndRenderPass();
	//}

	if (old_layout == new_layout)
	{
		pipeline_barrier.memory_src_stages |= src_stage_mask;
		pipeline_barrier.memory_dst_stages |= dst_stage_mask;

		pipeline_barrier.memory_src_access |= VK_Utils::AccessMaskFromImageLayout(old_layout,false);
		pipeline_barrier.memory_dst_access |= VK_Utils::AccessMaskFromImageLayout(new_layout,false);
	}

	// Check overlapping subresources
	for (size_t i = 0; i < image_barriers.size(); ++i)
	{
		CONST auto& ImgBarrier = image_barriers[i];
		if (ImgBarrier.image != image)
			continue;

		CONST auto& other_range = ImgBarrier.subresourceRange;

		CONST auto start_layer0 = subresource_range.baseArrayLayer;
		CONST auto end_layer0 = subresource_range.layerCount != VK_REMAINING_ARRAY_LAYERS ? (subresource_range.baseArrayLayer + subresource_range.layerCount) : ~0u;
		CONST auto start_layer1 = other_range.baseArrayLayer;
		CONST auto end_layer1 = other_range.layerCount != VK_REMAINING_ARRAY_LAYERS ? (other_range.baseArrayLayer + other_range.layerCount) : ~0u;

		CONST auto start_mip0 = subresource_range.baseMipLevel;
		CONST auto end_mip0 = subresource_range.levelCount != VK_REMAINING_MIP_LEVELS ? (subresource_range.baseMipLevel + subresource_range.levelCount) : ~0u;
		CONST auto start_mip1 = other_range.baseMipLevel;
		CONST auto end_mip1 = other_range.levelCount != VK_REMAINING_MIP_LEVELS ? (other_range.baseMipLevel + other_range.levelCount) : ~0u;

		CONST auto is_slices_overlap = CheckLineSectionOverlap<true>(start_layer0, end_layer0, start_layer1, end_layer1);
		CONST auto is_mips_overlap = CheckLineSectionOverlap<true>(start_mip0, end_mip0, start_mip1, end_mip1);

		// If the range overlaps with any of the existing barriers, we need to
		// flush them.
		if (is_slices_overlap && is_mips_overlap)
		{
			FlushBarriers();
			break;
		}
	}

	pipeline_barrier.image_src_stages |= src_stage_mask;
	pipeline_barrier.image_dst_stages |= dst_stage_mask;

	VkImageMemoryBarrier ImgBarrier{};
	ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImgBarrier.pNext = nullptr;
	ImgBarrier.oldLayout = old_layout;
	ImgBarrier.newLayout = new_layout;
	ImgBarrier.image = image;
	ImgBarrier.subresourceRange = subresource_range;
	ImgBarrier.srcAccessMask = VK_Utils::AccessMaskFromImageLayout(old_layout, false) & pipeline_barrier.supported_access_mask;
	ImgBarrier.dstAccessMask = VK_Utils::AccessMaskFromImageLayout(new_layout, true) & pipeline_barrier.supported_access_mask;
	ImgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // source queue family for a queue family ownership transfer.
	ImgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // destination queue family for a queue family ownership transfer.
	image_barriers.emplace_back(ImgBarrier);
}

void VK_CommandBuffer::FlushBarriers()
{
	if (state_cache.render_pass != VK_NULL_HANDLE)
	{
		EndRenderPass();
	}
	if(pipeline_barrier.memory_src_stages==0&&pipeline_barrier.memory_dst_stages==0&&image_barriers.empty())
	{
		return;
	}
	VkMemoryBarrier vk_mem_barrier{};
	vk_mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	vk_mem_barrier.pNext = nullptr;
	vk_mem_barrier.srcAccessMask = pipeline_barrier.memory_src_access & pipeline_barrier.supported_access_mask;
	vk_mem_barrier.dstAccessMask = pipeline_barrier.memory_dst_access & pipeline_barrier.supported_access_mask;

	CONST bool HasMemoryBarrier =
		pipeline_barrier.memory_src_stages != 0 && pipeline_barrier.memory_dst_stages != 0 &&
		pipeline_barrier.memory_src_access != 0 && pipeline_barrier.memory_dst_access != 0;

	CONST VkPipelineStageFlags SrcStages = (pipeline_barrier.image_src_stages | pipeline_barrier.memory_src_stages) & pipeline_barrier.supported_stages_mask;
	CONST VkPipelineStageFlags DstStages = (pipeline_barrier.image_dst_stages | pipeline_barrier.memory_dst_stages) & pipeline_barrier.supported_stages_mask;

	vkCmdPipelineBarrier(command_buffer,
		SrcStages,
		DstStages,
		0,
		HasMemoryBarrier ? 1 : 0,
		HasMemoryBarrier ? &vk_mem_barrier : nullptr,
		0,
		nullptr,
		static_cast<UInt32>(image_barriers.size()),
		image_barriers.empty() ? nullptr : image_barriers.data());

	image_barriers.clear();
	pipeline_barrier.image_src_stages = 0;
	pipeline_barrier.image_dst_stages = 0;
	pipeline_barrier.memory_src_stages = 0;
	pipeline_barrier.memory_dst_stages = 0;
	pipeline_barrier.memory_src_access = 0;
	pipeline_barrier.memory_dst_access = 0;
}

void VK_CommandBuffer::MemoryBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	if (state_cache.render_pass != VK_NULL_HANDLE)
	{
		EndRenderPass();
	}


	pipeline_barrier.memory_src_stages |= srcStageMask;
	pipeline_barrier.memory_dst_stages |= dstStageMask;

	pipeline_barrier.memory_src_access |= srcAccessMask;
	pipeline_barrier.memory_dst_access |= dstAccessMask;
}

void VK_CommandBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, VkImageLayout imageLayout,UInt32 region_count, CONST VkBufferImageCopy* region)
{
	if (state_cache.render_pass != VK_NULL_HANDLE)
	{
		EndRenderPass();
	}
	FlushBarriers();
	vkCmdCopyBufferToImage(command_buffer, buffer, image, imageLayout, region_count, region);
	
}


void VK_CommandBuffer::SetGraphicsPipeline(RenderPipelineState* pipeline_state)
{
	CHECK(pipeline_state == nullptr);
	VK_PipelineState * vk_pipeline_state = STATIC_CAST(pipeline_state, VK_PipelineState);
	VkPipeline graphics_pipeline = vk_pipeline_state->GetPipeline();
	VkPipelineLayout pipeline_layout = vk_pipeline_state->GetPipelineLayout();
	if (state_cache.graphics_pipeline != graphics_pipeline)
	{
		state_cache.graphics_pipeline = graphics_pipeline;
		state_cache.pipeline_layout = pipeline_layout;
	}
}
void VK_CommandBuffer::SetRenderTarget(CONST Vector<Texture*>& render_targets, Texture* depth_stencil, CONST Vector<ClearValue>& clear_values, Bool has_dsv_clear_value)
{
	CHECK_WITH_LOG(command_state < EState::IsInsideBegin, "RHI Error: SetRenderTarget must be called between Begin and End");
	Vector<ENUM_TEXTURE_FORMAT> rtv_formats;
	for (auto& rtv : render_targets)
	{
		rtv_formats.push_back(rtv->GetTextureDesc().format);
	}
	ENUM_TEXTURE_FORMAT depth_stencil_format = ENUM_TEXTURE_FORMAT::None;
	if(depth_stencil!=nullptr)
		depth_stencil_format = depth_stencil->GetTextureDesc().format;
	RenderPassCacheKey renderpass_key(render_targets.size(), rtv_formats.data(), depth_stencil_format, render_targets[0]->GetTextureDesc().samples, false, false);
	VK_RenderPass* vk_renderpass = device->GetRenderPassManager()->GetRenderPass(renderpass_key);

	VkRenderPass render_pass = vk_renderpass->GetRenderPass();

	FramebufferCacheKey framebuffer_key;
	framebuffer_key.render_targets = render_targets;
	framebuffer_key.depth_stencil = depth_stencil;
	framebuffer_key.render_pass = render_pass;
	VK_FrameBuffer* vk_framebuffer = device->GetFrameBufferManager()->GetFramebuffer(framebuffer_key, render_targets[0]->GetTextureDesc().width, render_targets[0]->GetTextureDesc().height, 1);

	VkFramebuffer framebuffer = vk_framebuffer->GetFramebuffer();
	
	TransitionRenderTargets(render_targets,depth_stencil);

	Vector<VkClearValue> vk_clear_values;
	Int size = clear_values.size() - 1;
	for (Int i = 0; size > 0 &&i < size;++i)
	{
		vk_clear_values.push_back( { clear_values[i].color[0], clear_values[i].color[1], clear_values[i].color[2], clear_values[i].color[3] });
	}
	if (has_dsv_clear_value&& size >0)
	{
		vk_clear_values.push_back( { clear_values[size].ds_value[0], clear_values[size].ds_value[1]});
	}
	if (state_cache.render_pass != render_pass || state_cache.framebuffer != framebuffer)
	{
		state_cache.render_targets = render_targets;
		state_cache.depth_stencil = depth_stencil;
	}
	BeginRenderPass(render_pass, framebuffer, render_targets[0]->GetTextureDesc().width, render_targets[0]->GetTextureDesc().height, vk_clear_values.size(),vk_clear_values.data());

}

void VK_CommandBuffer::SetShaderResourceBinding(ShaderResourceBinding* srb)
{
	CHECK(srb == nullptr);
	VK_ShaderResourceBinding* vk_srb = STATIC_CAST(srb, VK_ShaderResourceBinding);
	CONST VkDescriptorSet* descriptor_sets = vk_srb->GetDescriptorSets();
	for (UInt32 i = 0; i < MYRENDER_MAX_BINDING_SET_NUM; ++i)
	{
		if ( descriptor_sets[i] != VK_NULL_HANDLE)
		{
			state_cache.descriptor_sets = &(descriptor_sets[i]);
			state_cache.descriptor_sets_count = std::max((UInt8)(i+1), state_cache.descriptor_sets_count);
			break;
		}
	}
}

void VK_CommandBuffer::Draw(CONST DrawAttribute& draw_attr)
{
	
	VkViewport viewport{};
	viewport.x = 0.0f; viewport.y = 0.0f;
	viewport.width = state_cache.framebuffer_width;
	viewport.height = state_cache.framebuffer_height;
	viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { static_cast<UInt32>(state_cache.framebuffer_width), static_cast<UInt32>(state_cache.framebuffer_height) };
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state_cache.graphics_pipeline);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
	if(state_cache.descriptor_sets != nullptr)
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state_cache.pipeline_layout, 0, state_cache.descriptor_sets_count, state_cache.descriptor_sets, 0, nullptr);
	vkCmdDraw(command_buffer, draw_attr.vertexCount, draw_attr.instanceCount, draw_attr.firstVertex, draw_attr.firstInstance);
}

VK_CommandBufferManager::~VK_CommandBufferManager()
{
	for (UInt32 index = 0; index < graphic_cmd_buffers.size(); ++index)
	{
		delete graphic_cmd_buffers[index];
	}
	for (UInt32 index = 0; index < compute_cmd_buffers.size(); ++index)
	{
		delete compute_cmd_buffers[index];
	}
	for (UInt32 index = 0; index < transfer_cmd_buffers.size(); ++index)
	{
		delete transfer_cmd_buffers[index];
	}
}

VK_CommandBufferManager::VK_CommandBufferManager(VK_Device* in_device):device(in_device)
{
	graphic_cmd_pools.emplace_back(in_device,*this);
	graphic_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().graphics_family.value());
	compute_cmd_pools.emplace_back(in_device, *this);
	compute_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().compute_family.value());
	transfer_cmd_pools.emplace_back(in_device, *this);
	transfer_cmd_pools[0].Init(in_device->GetQueueFamilyIndices().transfer_family.value());

	//graphic_cmd_buffers.push_back(std::move(graphic_cmd_pools[0].GetOrCreateCommandBuffer(false)));
	//compute_cmd_buffers.push_back(std::move(compute_cmd_pools[0].GetOrCreateCommandBuffer(false)));
	//transfer_cmd_buffers.push_back(std::move(transfer_cmd_pools[0].GetOrCreateCommandBuffer(false)));

}

VK_CommandBuffer* VK_CommandBufferManager::GetOrCreateCommandBuffer(ENUM_QUEUE_TYPE queue_type,Bool is_upload_only)
{
	switch (queue_type)
	{
	case ENUM_QUEUE_TYPE::GRAPHICS :
		return graphic_cmd_pools[0].GetOrCreateCommandBuffer(is_upload_only);
	case ENUM_QUEUE_TYPE::COMPUTE:
		return compute_cmd_pools[0].GetOrCreateCommandBuffer(is_upload_only);
	case ENUM_QUEUE_TYPE::TRANSFER:
		return transfer_cmd_pools[0].GetOrCreateCommandBuffer(is_upload_only);
	default:
		return nullptr;
	}
}

void VK_CommandBufferManager::ReleaseCommandBuffer(VK_CommandBuffer* cmd_buffer)
{
	//cmd_buffer->GetFence()->ResetFence();
	cmd_buffer->owner_pool->FreeUnusedCommandBuffer(cmd_buffer);
}

void VK_CommandBufferManager::FreeUnusedCommandBuffer(ENUM_QUEUE_TYPE queue_type)
{
	switch (queue_type)
	{
	case ENUM_QUEUE_TYPE::GRAPHICS:
		return graphic_cmd_pools[0].FreeUnusedCommandBuffers(nullptr);
	case ENUM_QUEUE_TYPE::COMPUTE:
		return compute_cmd_pools[0].FreeUnusedCommandBuffers(nullptr);
	case ENUM_QUEUE_TYPE::TRANSFER:
		return transfer_cmd_pools[0].FreeUnusedCommandBuffers(nullptr);
	default:
		return ;
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE