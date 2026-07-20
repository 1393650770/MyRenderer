#pragma once
#ifndef _RENDERINTERFACE_
#define _RENDERINTERFACE_
#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraph.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
class PlatformWindow;
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
class Texture;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Render)
struct FrameContext;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_CLASS(RenderInterface)

#pragma region METHOD

public:
	RenderInterface() MYDEFAULT;
	VIRTUAL ~RenderInterface() MYDEFAULT;

	// ========== Logic Thread Lifecycle ==========
	VIRTUAL void METHOD(OnInit_Logic)(PlatformWindow* in_window, RHI::Viewport* in_viewport) {}
	VIRTUAL void METHOD(OnShutdown_Logic)() {}

	// ========== Render Thread Lifecycle ==========
	VIRTUAL void METHOD(OnInit_Render)() {}
	VIRTUAL void METHOD(OnShutdown_Render)() {}

	// ========== Logic Thread Per-Frame ==========
	VIRTUAL void METHOD(OnUpdate)(float dt) {}
	VIRTUAL void METHOD(OnPrepareFrameContext)(Render::FrameContext& ctx) {}

	// ========== Render Thread Per-Frame ==========
	VIRTUAL void METHOD(OnPreRender)(Render::FrameContext& ctx) {}
	VIRTUAL void METHOD(OnRender)() PURE;
	VIRTUAL void METHOD(OnPostRender)(Render::FrameContext& ctx) {}

	// ========== Backward compat wrappers (Single / RHIThread modes) ==========
	VIRTUAL void METHOD(OnInit)(PlatformWindow* in_window, RHI::Viewport* in_viewport) FINAL;
	VIRTUAL void METHOD(OnShutdown)() FINAL;

	// === Runtime-managed graph access ===
	Render::RenderGraph& METHOD(GetRenderGraph)() { return graph; }

protected:

private:

#pragma endregion

#pragma region MEMBER

protected:
	Render::RenderGraph graph;

private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE

#endif
