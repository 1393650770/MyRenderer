#pragma once
#ifndef _RENDERINTERFACE_
#define _RENDERINTERFACE_
#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraph.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)
class Window;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_CLASS(RenderInterface)

#pragma region METHOD

public:
	RenderInterface() MYDEFAULT;
	VIRTUAL ~RenderInterface() MYDEFAULT;

	// === Lifecycle (called on main thread) ===
	VIRTUAL void METHOD(OnInit)(Application::Window* window) PURE;
	VIRTUAL void METHOD(OnShutdown)() PURE;

	// === Per-frame (called on LogicThread) ===
	VIRTUAL void METHOD(OnUpdate)(float dt) {}

	// === Per-frame (called on RenderThread) ===
	VIRTUAL void METHOD(OnRender)() PURE;

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
