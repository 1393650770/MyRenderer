#include "VK_FrameBuffer.h"
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_RenderPass.h"
#include "VK_Texture.h"
#include "Core/TypeHash.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


VK_FrameBuffer::~VK_FrameBuffer()
{
	if (framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device->GetDevice(), framebuffer, nullptr);
		framebuffer = VK_NULL_HANDLE;
	}
}

VK_FrameBuffer::VK_FrameBuffer(VK_Device* in_device, CONST FrameBufferDesc& in_desc, VkRenderPass in_render_pass) : FrameBuffer(in_desc), device(in_device), render_pass(in_render_pass)
{
	CHECK_WITH_LOG(in_render_pass == nullptr, "RHI Error : render pass is nullptr when to create framebuffer!");
	VK_RenderPassManager* render_pass_manager = device->GetRenderPassManager();
	VkFramebufferCreateInfo framebuffer_create_info{};
	framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//framebuffer_create_info.renderPass = render_pass_manager->GetRenderPass(in_desc.render_pass_key);
	//framebuffer_create_info.attachmentCount = in_desc.attachments.size();
	//framebuffer_create_info.pAttachments = in_desc.attachments.data();
	framebuffer_create_info.width = in_desc.width;
	framebuffer_create_info.height = in_desc.height;
	framebuffer_create_info.layers = in_desc.layer;
	framebuffer_create_info.renderPass = in_render_pass;
	Vector<VkImageView> attachments;
	for (CONST auto& render_target : in_desc.render_targets)
	{
		attachments.push_back(STATIC_CAST(render_target,VK_Texture)->GetImageView());
	}
	if (in_desc.depth_stencil_view)
		attachments.push_back(STATIC_CAST(in_desc.depth_stencil_view, VK_Texture)->GetImageView());
	framebuffer_create_info.attachmentCount = attachments.size();
	framebuffer_create_info.pAttachments = attachments.data();
	CHECK_WITH_LOG(vkCreateFramebuffer(device->GetDevice(), &framebuffer_create_info, nullptr, &framebuffer) != VK_SUCCESS,("RHI Error : failed to create framebuffer!"));


}

VkFramebuffer VK_FrameBuffer::GetFramebuffer() CONST
{
	return framebuffer;
}

VK_FrameBufferManager::~VK_FrameBufferManager()
{
	Destroy();
}

VK_FrameBufferManager::VK_FrameBufferManager(VK_Device* in_device) : device(in_device)
{

}

VK_FrameBuffer* VK_FrameBufferManager::GetFramebuffer(CONST FramebufferCacheKey& key, uint32_t width, uint32_t height, uint32_t layers)
{
	auto it = framebuffer_cache.find(key);
	if (it != framebuffer_cache.end())
	{
		return it->second;
	}
	else
	{
		
		FrameBufferDesc frame_buffer_desc;
		frame_buffer_desc.render_targets = key.render_targets;
		frame_buffer_desc.width = width;
		frame_buffer_desc.height = height;
		frame_buffer_desc.layer = layers;
		frame_buffer_desc.depth_stencil_view = key.depth_stencil;
		
		VK_FrameBuffer* frame_buffer = new VK_FrameBuffer(device, frame_buffer_desc, key.render_pass);

		auto new_it = framebuffer_cache.insert(std::make_pair(key, std::move(frame_buffer)));

		render_pass_to_key_map.emplace(key.render_pass, key);
		if (key.depth_stencil != nullptr)
			view_to_key_map.emplace(std::make_pair(((VK_Texture*)key.depth_stencil)->GetImageView(), key));
		for (UInt32 rt = 0; rt < key.render_targets.size(); ++rt)
			if (key.render_targets[rt] != nullptr)
				view_to_key_map.emplace(std::make_pair(((VK_Texture*)key.render_targets[rt])->GetImageView(), key));

		return frame_buffer;
	}
}

void VK_FrameBufferManager::OnDestroyImageView(VkImageView image_view)
{
	auto it = view_to_key_map.find(image_view);
	if (it != view_to_key_map.end())
	{
		auto key_it = framebuffer_cache.find(it->second);
		if (key_it != framebuffer_cache.end())
		{
			delete key_it->second;
			framebuffer_cache.erase(key_it);
		}
		view_to_key_map.erase(it);
	}
}

void VK_FrameBufferManager::OnDestroyRenderPass(VkRenderPass render_pass)
{
	auto it = render_pass_to_key_map.find(render_pass);
	if (it != render_pass_to_key_map.end())
	{
		auto key_it = framebuffer_cache.find(it->second);
		if (key_it != framebuffer_cache.end())
		{
			delete key_it->second;
			framebuffer_cache.erase(key_it);
		}
		render_pass_to_key_map.erase(it);
	}
}

void VK_FrameBufferManager::Destroy()
{
	for (auto& it : framebuffer_cache)
	{
		delete it.second;
	}
	framebuffer_cache.clear();

}

Bool FramebufferCacheKey::operator==(CONST FramebufferCacheKey& rhs) CONST
{
	if (GetHash() != rhs.GetHash() ||
		render_pass != rhs.render_pass ||
		render_targets.size() != rhs.render_targets.size() ||
		depth_stencil != rhs.depth_stencil ||
		shading_rate != rhs.shading_rate ||
		command_queue_mask != rhs.command_queue_mask)
	{
		return false;
	}

	for (UInt32 rt = 0; rt < render_targets.size(); ++rt)
		if (render_targets[rt] != rhs.render_targets[rt])
			return false;

	return true;
}

UInt64 FramebufferCacheKey::GetHash() CONST
{
	if (hash == 0)
	{
		HashCombine(hash,render_pass, render_targets.size(), depth_stencil, shading_rate, command_queue_mask);
		for (UInt32 rt = 0; rt < render_targets.size(); ++rt)
			HashCombine(hash, render_targets[rt]);
	}
	return hash;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE