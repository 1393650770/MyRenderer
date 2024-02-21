#include "RenderGraphResourceImplementation.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

template<>
std::unique_ptr<MXRender::RHI::Texture> Realize<MXRender::RHI::TextureDesc, MXRender::RHI::Texture>(CONST MXRender::RHI::TextureDesc& description)
{
	std::unique_ptr<MXRender::RHI::Texture> texture(RHICreateTexture(description));
	return std::move(texture);
}

template<>
std::unique_ptr<MXRender::RHI::Buffer> Realize<MXRender::RHI::BufferDesc, MXRender::RHI::Buffer>(CONST MXRender::RHI::BufferDesc& description)
{
	std::unique_ptr<MXRender::RHI::Buffer> buffer(RHICreateBuffer(description));
	return std::move(buffer);
}
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
