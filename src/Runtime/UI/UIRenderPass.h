
#pragma once
#ifndef _UIRENDERPASS_
#define _UIRENDERPASS_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraph.h"

// Forward declarations
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class UIRenderer;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
class Texture;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

/**
 * RDG pass data for UI rendering.
 *
 * Owns the PSO for the optional composite pass (Mode B: offscreen→backbuffer).
 */
struct UIPassData : public Render::RenderGraphPassDataBase {
	RHI::RenderPipelineState* composite_pso = nullptr;
	VIRTUAL ~UIPassData() { Release(); }
	VIRTUAL void Release() OVERRIDE {
		// composite_pso is owned by the pipeline state manager — do NOT delete
		composite_pso = nullptr;
	}
};

/**
 * Register RmlUI rendering as an RDG pass.
 *
 * Automatically selects Mode A (direct backbuffer write) or Mode B
 * (offscreen + composite) based on NeedsOffscreen().
 *
 * @param graph           The RenderGraph to add passes to
 * @param bb_resource     Retained backbuffer resource ("BackBuffer")
 * @param renderer        The UI renderer backend
 * @param ui_draw_fn      Callback that records UI draw commands (e.g. Rml::Context::Render())
 * @param viewport_w, viewport_h  Current viewport dimensions
 */
void RegisterUIPass(
	Render::RenderGraph* graph,
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* bb_resource,
	UIRenderer* renderer,
	RHI::CommandList* cmd_list,
	UInt32 viewport_w, UInt32 viewport_h);

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_UIRENDERPASS_
