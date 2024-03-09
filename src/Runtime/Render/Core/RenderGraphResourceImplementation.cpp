#include "RenderGraphResourceImplementation.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderPass.h"
#include "RHI/RenderFrameBuffer.h"
#include "RHI/RenderPipelineState.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

template<>
std::unique_ptr<MXRender::RHI::Texture> RealizeResource<MXRender::RHI::TextureDesc, MXRender::RHI::Texture>(CONST MXRender::RHI::TextureDesc& description)
{
	std::unique_ptr<MXRender::RHI::Texture> texture(RHICreateTexture(description));
	return std::move(texture);
}

template<>
std::unique_ptr<MXRender::RHI::Buffer> RealizeResource<MXRender::RHI::BufferDesc, MXRender::RHI::Buffer>(CONST MXRender::RHI::BufferDesc& description)
{
	std::unique_ptr<MXRender::RHI::Buffer> buffer(RHICreateBuffer(description));
	return std::move(buffer);
}
/*
template<>
std::unique_ptr<MXRender::RHI::RenderPass> RealizeResource<MXRender::RHI::RenderPassDesc, MXRender::RHI::RenderPass>(CONST MXRender::RHI::RenderPassDesc& description)
{
	std::unique_ptr<MXRender::RHI::RenderPass> render_pass(RHICreateRenderPass(description));;
	return std::move(render_pass);
}

template<>
std::unique_ptr<MXRender::RHI::FrameBuffer> RealizeResource<MXRender::RHI::FrameBufferDesc, MXRender::RHI::FrameBuffer>(CONST MXRender::RHI::FrameBufferDesc& description)
{
	std::unique_ptr<MXRender::RHI::FrameBuffer> frame_buffer(RHICreateFrameBuffer(description));
	return std::move(frame_buffer);
}
*/
template<>
std::unique_ptr<MXRender::RHI::RenderPipelineState> RealizeResource<MXRender::RHI::RenderGraphiPipelineStateDesc, MXRender::RHI::RenderPipelineState>(CONST MXRender::RHI::RenderGraphiPipelineStateDesc& description)
{
	std::unique_ptr<MXRender::RHI::RenderPipelineState> render_pipeline_state(RHICreateRenderPipelineState(description));
	return std::move(render_pipeline_state);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
