#pragma once
#ifndef _RENDER_FRAMEDATA_
#define _RENDER_FRAMEDATA_

#include "Core/ConstDefine.h"
#include "Core/ConstGlobals.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Render)

// Forward declare RenderGraphDefinition (defined in Render layer)
class RenderGraphDefinition;

// -- [AI] Per-frame data passed from Logic thread to Render thread
// Backbuffer RTV/DSV acquired on Render thread (via OnPreRender) to avoid race with Present.
MYRENDERER_BEGIN_STRUCT(FrameContext)
public:
	Float32 deltaTime = 0.0f;
	UInt32 viewport_width = 0;
	UInt32 viewport_height = 0;
	UInt64 frame_number = 0;
	// ImGui context pointer (Logic saves after NewFrame, Render restores)
	void* imgui_context = nullptr;
	// Viewport resize request from Logic thread
	Bool needs_resize = false;
	Int resize_width = 0;
	Int resize_height = 0;
	// Deferred RenderGraph rebuild from Logic thread
	Bool has_rebuild = false;
	RenderGraphDefinition* rebuild_def = nullptr;
	// ImGui draw data: Logic thread generates, Render thread records & deletes
	// ImGui draw data (void* to avoid Runtime depending on imgui.h)
	void* draw_data = nullptr;
MYRENDERER_END_STRUCT

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
