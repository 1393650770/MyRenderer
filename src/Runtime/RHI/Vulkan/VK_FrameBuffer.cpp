#include "VK_FrameBuffer.h"
#include "vulkan/vulkan_core.h"
#include "VK_Device.h"
#include "VK_RenderPass.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)


VK_FrameBuffer::~VK_FrameBuffer()
{

}

VK_FrameBuffer::VK_FrameBuffer(VK_Device* in_device, CONST FrameBufferDesc& in_desc) : FrameBuffer(in_desc), device(in_device)
{
	VK_RenderPassManager* render_pass_manager = device->GetRenderPassManager();
	VkFramebufferCreateInfo framebuffer_create_info{};
	framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//framebuffer_create_info.renderPass = render_pass_manager->GetRenderPass(in_desc.render_pass_key);
	//framebuffer_create_info.attachmentCount = in_desc.attachments.size();
	//framebuffer_create_info.pAttachments = in_desc.attachments.data();
	framebuffer_create_info.width = in_desc.width;
	framebuffer_create_info.height = in_desc.height;
	framebuffer_create_info.layers = in_desc.layer;
	//if (vkCreateFramebuffer(device->GetDevice(), &framebuffer_create_info, nullptr, &framebuffer) != VK_SUCCESS) {
	//	throw std::runtime_error("failed to create framebuffer!");
	//}

}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE