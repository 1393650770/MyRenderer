#pragma once

#ifndef _RENDERRHI_
#define _RENDERRHI_
#include"RenderEnum.h"
#include <string>
#include<vector>
#include<memory>
#include "Core/ConstDefine.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
class Viewport;
class Buffer;
class Shader;

MYRENDERER_BEGIN_CLASS(RenderFactory)
public:
	Int render_api_version=0;
	Bool enable_render_debug=false;

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderRHI,public RenderResource)

public:
	RenderRHI() DEFAULT;
	VIRTUAL~ RenderRHI() DEFAULT;

#pragma region INIT_MATHOD
	VIRTUAL void METHOD(Init)(RenderFactory* render_factory) PURE;

	VIRTUAL void METHOD(PostInit)() PURE;

	VIRTUAL void METHOD(Shutdown)() PURE;

#pragma endregion


#pragma region CREATE_RESOURCE
	VIRTUAL Viewport* METHOD(CreateViewport)(void* window_handle, Int width, Int height, Bool is_full_screen) PURE;
	VIRTUAL Shader* METHOD(CreateShader)(CONST ShaderDesc& desc, CONST ShaderDataPayload& data) PURE;
	VIRTUAL Buffer* METHOD(CreateBuffer)(const BufferDesc& buffer_desc) PURE;
	VIRTUAL void* METHOD(MapBuffer)(Buffer* buffer) PURE;
	VIRTUAL void METHOD(UnmapBuffer)(Buffer* buffer) PURE;


#pragma endregion

#pragma region DRAW

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

#endif

