#pragma once
#ifndef _UIRENDERPASS_
#define _UIRENDERPASS_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraph.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
class Texture;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class UIRenderer;

struct UIPassData : public Render::RenderGraphPassDataBase {
	VIRTUAL ~UIPassData() { Release(); }
	VIRTUAL void Release() OVERRIDE {}
};

/// Register UI rendering as an RDG pass.
/// @param draw_fn  Called between BeginFrame/EndFrame to record UI draw commands
void RegisterUIPass(
	Render::RenderGraph* graph,
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* bb_resource,
	UIRenderer* renderer,
	RHI::CommandList* cmd_list,
	UInt32 viewport_w, UInt32 viewport_h,
	std::function<void(RHI::CommandList*)> draw_fn);

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_UIRENDERPASS_
