#include "RenderRHI.h"
#include "Platform/Platform.h"
#include "Vulkan/VK_RenderPass.h"
#include "Vulkan/VK_RenderRHI.h"
#include "Vulkan/VK_BindlessManager.h"
#include "RenderTexture.h"

MXRender::RHI::RenderRHI* g_render_rhi = nullptr;

void RHIInit()
{
	if (g_render_rhi == nullptr)
	{
		g_render_rhi = PlatformCreateDynamicRHI();
	}
}

void RHIShutdown()
{


	if (g_render_rhi != nullptr)
	{
		g_render_rhi->Shutdown();
		delete g_render_rhi;
		g_render_rhi = nullptr;
	}
}

MXRender::RHI::Viewport* RHICreateViewport(void* window_handle, Int width, Int height, Bool is_full_screen)
{
	return g_render_rhi->CreateViewport(window_handle, width, height, is_full_screen);
}

MXRender::RHI::Texture* RHICreateTexture(CONST MXRender::RHI::TextureDesc& texture_desc)
{
	return g_render_rhi->CreateTexture(texture_desc);
}

MXRender::RHI::Buffer* RHICreateBuffer(CONST MXRender::RHI::BufferDesc& buffer_desc)
{
	return g_render_rhi->CreateBuffer(buffer_desc);
}

MXRender::RHI::Shader* RHICreateShader(CONST MXRender::RHI::ShaderDesc& desc, CONST MXRender::RHI::ShaderDataPayload& data)
{
	return g_render_rhi->CreateShader(desc,data);
}

MXRender::RHI::RenderPipelineState* RHICreateRenderPipelineState(CONST MXRender::RHI::RenderGraphiPipelineStateDesc& desc)
{
	return g_render_rhi->CreateRenderPipelineState(desc);
}

// --  
MXRender::RHI::ComputePipelineState* RHICreateComputePipelineState(CONST MXRender::RHI::ComputePipelineStateDesc& desc)
{
	return g_render_rhi->CreateComputePipelineState(desc);
}

MXRender::RHI::RenderPass* RHICreateRenderPass(CONST MXRender::RHI::RenderPassDesc& desc)
{
	return g_render_rhi->CreateRenderPass(desc);
}

MXRender::RHI::FrameBuffer* RHICreateFrameBuffer(CONST MXRender::RHI::FrameBufferDesc& desc)
{
	return g_render_rhi->CreateFrameBuffer(desc);
}

MXRender::RHI::CommandList* RHIGetImmediateCommandList()
{
 	return g_render_rhi->GetImmediateCommandList();
}

// --  
MXRender::RHI::CommandList* RHIGetCommandListForQueue(MXRender::ENUM_QUEUE_TYPE queue_type)
{
	return g_render_rhi->GetCommandListForQueue(queue_type);
}

void RHISubmitCommandList(MXRender::RHI::CommandList* command_list)
{
	g_render_rhi->SubmitCommandList(command_list);
}

// --  
void RHISubmitCommandListForQueue(MXRender::RHI::CommandList* cmd_list, MXRender::ENUM_QUEUE_TYPE queue_type)
{
	g_render_rhi->SubmitCommandListForQueue(cmd_list, queue_type);
}

void RHIRenderEnd()
{
	g_render_rhi->RenderEnd();
}

void* RHIMapBuffer(MXRender::RHI::Buffer* buffer, MXRender::ENUM_MAP_TYPE map_type, MXRender::ENUM_MAP_FLAG map_flag)
{
	return g_render_rhi->MapBuffer(buffer, map_type,map_flag);
}

void RHIUnmapBuffer(MXRender::RHI::Buffer* buffer)
{
	g_render_rhi->UnmapBuffer(buffer);
}

MXRender::RHI::Vulkan::VK_BindlessManager* RHIGetBindlessManager()
{
	if (g_render_rhi)
	{
		auto* vulkan_rhi = static_cast<MXRender::RHI::Vulkan::VulkanRHI*>(g_render_rhi);
		return vulkan_rhi->GetBindlessManager();
	}
	return nullptr;
}

