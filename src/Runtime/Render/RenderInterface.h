#pragma once
#ifndef _RENDERINTERFACE_
#define _RENDERINTERFACE_
#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraph.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)
class Window;
MYRENDERER_END_NAMESPACE
MYRENDERER_BEGIN_NAMESPACE(RHI)
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
	VIRTUAL void METHOD(OnInit_Logic)(Application::Window* window) {}
	VIRTUAL void METHOD(OnShutdown_Logic)() {}

	// ========== Render Thread Lifecycle ==========
	VIRTUAL void METHOD(OnInit_Render)() {}
	VIRTUAL void METHOD(OnShutdown_Render)() {}

	// ========== Logic Thread Per-Frame ==========
	VIRTUAL void METHOD(OnUpdate)(float dt) {}
	// Fill FrameContext with Logic thread data (deferred rebuilds, etc.)
	VIRTUAL void METHOD(OnPrepareFrameContext)(Render::FrameContext& ctx) {}

	// ========== Render Thread Per-Frame ==========
	// Called before command recording: rebuild graph, update backbuffer pointers
	VIRTUAL void METHOD(OnPreRender)(Render::FrameContext& ctx) {}
	// Record draw commands
	VIRTUAL void METHOD(OnRender)() PURE;
	// Called after Present: frame stats, cleanup
	VIRTUAL void METHOD(OnPostRender)(Render::FrameContext& ctx) {}

	// ========== Backward compat wrappers (Single / RHIThread modes) ==========
	// Internal: calls OnInit_Logic + OnInit_Render
	VIRTUAL void METHOD(OnInit)(Application::Window* window) FINAL;
	// Internal: calls OnShutdown_Logic + OnShutdown_Render
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
