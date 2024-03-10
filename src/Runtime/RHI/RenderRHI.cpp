#include "RenderRHI.h"
#include "Platform/Platform.h"
#include "Vulkan/VK_RenderPass.h"
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

void RHISubmitCommandList(MXRender::RHI::CommandList* command_list)
{
	g_render_rhi->SubmitCommandList(command_list);
}