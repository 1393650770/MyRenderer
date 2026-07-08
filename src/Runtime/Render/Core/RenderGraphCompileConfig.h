#pragma once
#ifndef _RENDERGRAPHCOMPILECONFIG_
#define _RENDERGRAPHCOMPILECONFIG_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// --   --
// Configuration flags for RenderGraph::Compile().
// Controls which compile phases are enabled.
// Safe mode (fallback_to_conservative) applies full barriers + no aliasing + serial.
MYRENDERER_BEGIN_STRUCT(RenderGraphCompileConfig)
public:
	Bool enable_barrier_generation = true;
	Bool enable_memory_aliasing = true;
	Bool enable_async_compute = true;
	Bool fallback_to_conservative = false;

	// Debug toggles
	Bool debug_dump_graphviz = false;
	Bool debug_log_transitions = false;
MYRENDERER_END_STRUCT
// --   --

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHCOMPILECONFIG_
