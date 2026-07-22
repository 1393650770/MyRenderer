
#pragma once
#ifndef _UIDRAWBUFFER_
#define _UIDRAWBUFFER_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

/**
 * Per-frame draw buffer.
 *
 * Accumulates draw commands from the UI system during Update/Paint,
 * then the renderer replays them during the RDG pass execute lambda.
 *
 * v1: Simple — the UIRenderer concrete implementation directly records
 * RHI commands during DrawGeometry() rather than deferred batching.
 * This struct is a placeholder for future batching (sort by texture, layer, etc.)
 * following UE's FSlateDrawBuffer pattern.
 */
struct UIDrawStats {
	UInt32 geometry_compiled = 0;   // geometries uploaded this frame
	UInt32 draw_calls = 0;          // DrawGeometry calls this frame
	UInt32 textures_created = 0;    // textures created this frame
	bool   used_clip_mask = false;   // stencil clip mask was active
	bool   used_layers = false;      // offscreen layer stack was used
	bool   used_transform = false;   // SetTransform was called
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_UIDRAWBUFFER_
