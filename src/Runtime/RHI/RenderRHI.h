#pragma once

#ifndef _RENDERRHI_
#define _RENDERRHI_
#include"RenderEnum.h"
#include <string>
#include<vector>
#include<memory>
#include "RenderRource.h"
#include "Core/ConstGlobals.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
class Viewport;
class Buffer;
class Shader;
class RenderPipelineState;
class RenderPass;
class FrameBuffer;
class CommandList;
MYRENDERER_BEGIN_CLASS(RenderFactory)
public:
	Int render_api_version=0;
	Bool enable_render_debug=false;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderRHI,public RenderResource)

public:
	RenderRHI() MYDEFAULT;
	VIRTUAL~ RenderRHI() MYDEFAULT;

#pragma region INIT_MATHOD
	VIRTUAL void METHOD(Init)(RenderFactory* render_factory) PURE;

	VIRTUAL void METHOD(PostInit)() PURE;

	VIRTUAL void METHOD(Shutdown)() PURE;

#pragma endregion


#pragma region CREATE_RESOURCE
	VIRTUAL Viewport* METHOD(CreateViewport)(void* window_handle, Int width, Int height, Bool is_full_screen) PURE;
	VIRTUAL Shader* METHOD(CreateShader)(CONST ShaderDesc& desc, CONST ShaderDataPayload& data) PURE;
	VIRTUAL Buffer* METHOD(CreateBuffer)(CONST BufferDesc& buffer_desc) PURE;
	VIRTUAL Texture* METHOD(CreateTexture)(CONST TextureDesc& texture_desc) PURE;
	VIRTUAL RenderPipelineState* METHOD(CreateRenderPipelineState)(CONST RenderGraphiPipelineStateDesc& desc) PURE;
	VIRTUAL RenderPass* METHOD(CreateRenderPass)(CONST RenderPassDesc& desc) PURE;
	VIRTUAL FrameBuffer* METHOD(CreateFrameBuffer)(CONST FrameBufferDesc& desc) PURE;

	VIRTUAL void* METHOD(MapBuffer)(Buffer* buffer) PURE;
	VIRTUAL void METHOD(UnmapBuffer)(Buffer* buffer) PURE;

#pragma endregion

#pragma region DRAW
	VIRTUAL	CommandList* METHOD(GetImmediateCommandList)() PURE;
	VIRTUAL void METHOD(SubmitCommandList)(CommandList* command_list) PURE;
#pragma endregion

private:

protected:


MYRENDERER_END_CLASS



MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
extern CORE_API MXRender::RHI::RenderRHI* g_render_rhi;

extern CORE_API void METHOD(RHIInit)();
extern CORE_API void METHOD(RHIShutdown)();

extern CORE_API MXRender::RHI::Viewport* METHOD(RHICreateViewport)(void* window_handle, Int width, Int height, Bool is_full_screen);
extern CORE_API MXRender::RHI::Texture* METHOD(RHICreateTexture)(CONST MXRender::RHI::TextureDesc& texture_desc);
extern CORE_API MXRender::RHI::Buffer* METHOD(RHICreateBuffer)(CONST MXRender::RHI::BufferDesc& buffer_desc);
extern CORE_API MXRender::RHI::Shader* METHOD(RHICreateShader)(CONST MXRender::RHI::ShaderDesc& desc, CONST MXRender::RHI::ShaderDataPayload& data);
extern CORE_API MXRender::RHI::RenderPipelineState* METHOD(RHICreateRenderPipelineState)(CONST MXRender::RHI::RenderGraphiPipelineStateDesc& desc);
extern CORE_API MXRender::RHI::RenderPass* METHOD(RHICreateRenderPass)(CONST MXRender::RHI::RenderPassDesc& desc);
extern CORE_API MXRender::RHI::FrameBuffer* METHOD(RHICreateFrameBuffer)(CONST MXRender::RHI::FrameBufferDesc& desc);
extern CORE_API MXRender::RHI::CommandList* METHOD(RHIGetImmediateCommandList)();
extern CORE_API MXRender::RHI::CommandList* METHOD(RHIGetImmediateCommandList)();
extern CORE_API void METHOD(RHISubmitCommandList)(MXRender::RHI::CommandList* command_list);
#endif

