#include "VK_RenderPass.h"
#include "VK_Utils.h"
#include "VK_FrameBuffer.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


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
		UInt32 num_rt = key.num_render_targets + 1;
		if (key.depth_stencil_format == ENUM_TEXTURE_FORMAT::None)
		{
			num_rt -= 1;
			desc.last_attachment_is_depth_stencil = false;
		}
		desc.attachments.resize(num_rt);
		desc.attachment_refs.resize(num_rt);
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
		if (key.depth_stencil_format != ENUM_TEXTURE_FORMAT::None)
		{
			desc.attachments[key.num_render_targets].format = key.depth_stencil_format;
			desc.attachments[key.num_render_targets].sample_count = key.sample_count;
			desc.attachments[key.num_render_targets].load_op = ENUM_RENDERPASS_ATTACHMENT_LOAD_OP::Load;
			desc.attachments[key.num_render_targets].store_op = ENUM_RENDERPASS_ATTACHMENT_STORE_OP::Store;
			ENUM_RESOURCE_STATE depth_state = key.is_read_only_dsv ? ENUM_RESOURCE_STATE::DepthRead : ENUM_RESOURCE_STATE::DepthWrite;
			desc.attachments[key.num_render_targets].initial_state = depth_state;
			desc.attachments[key.num_render_targets].final_state = depth_state;
			desc.attachment_refs[key.num_render_targets].attachment_index = key.num_render_targets;
			desc.attachment_refs[key.num_render_targets].state = depth_state;
		}
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
	if (desc.last_attachment_is_depth_stencil)
	{
		subpass.colorAttachmentCount = attachment_ref.size() - 1;
		subpass.pColorAttachments = attachment_ref.data();
		subpass.pDepthStencilAttachment = &attachment_ref[attachment_ref.size() - 1];
	}
	else
	{
		subpass.colorAttachmentCount = attachment_ref.size();
		subpass.pColorAttachments = attachment_ref.data();
		subpass.pDepthStencilAttachment = nullptr;
	}
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ;

	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = attachment.size();
	render_pass_info.pAttachments = attachment.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies =
		&dependency;
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
		device->GetFrameBufferManager()->OnDestroyRenderPass(render_pass);
		vkDestroyRenderPass(device->GetDevice(), render_pass, nullptr);
		render_pass = VK_NULL_HANDLE;
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
