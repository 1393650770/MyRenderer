
#pragma once
#ifndef _UIRENDERER_
#define _UIRENDERER_

#include "Core/ConstDefine.h"
#include "UI/UIHandleTypes.h"

// Forward-declare RHI types (no Vulkan dependency)
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

/**
 * Abstract UI rendering backend.
 *
 * This interface provides a type-erased rendering API that any UI system
 * (RmlUI, ImGui, future libraries) can use to submit GPU draw commands.
 * The concrete implementation owns the Vulkan resources (shaders, PSOs,
 * vertex/index buffer pools, texture caches) and translates calls into
 * RHI CommandList operations.
 *
 * Design patterned after UE's FSlateRenderer / ISlateRenderer.
 */
MYRENDERER_BEGIN_CLASS(UIRenderer)

#pragma region METHOD
public:
	VIRTUAL ~UIRenderer() MYDEFAULT;

	/// Called once per frame before any draw calls.
	/// Binds the command list and resets internal state (draw buffer, layer stack).
	VIRTUAL void METHOD(BeginFrame)(RHI::CommandList* cmd) PURE;

	/// Called once per frame after all draw calls.
	/// Ensures all pending batches are flushed to the command list.
	VIRTUAL void METHOD(EndFrame)(RHI::CommandList* cmd) PURE;

	// -----------------------------------------------------------------------
	// Geometry management
	// -----------------------------------------------------------------------

	/// Compile vertex/index data into a GPU-resident geometry handle.
	/// @param vertices    Raw vertex buffer (layout defined by concrete impl)
	/// @param vtx_count   Number of vertices
	/// @param vtx_stride  Size of each vertex in bytes
	/// @param indices     Raw index buffer (may be nullptr for non-indexed)
	/// @param idx_count   Number of indices
	/// @param idx32       True if 32-bit indices, false for 16-bit
	/// @return Opaque geometry handle (>0), or 0 on failure
	VIRTUAL UIGeometryHandle METHOD(CompileGeometry)(
		CONST void* vertices, UInt32 vtx_count, UInt32 vtx_stride,
		CONST void* indices,  UInt32 idx_count, bool idx32) PURE;

	VIRTUAL void METHOD(DrawGeometry)(
		UIGeometryHandle geo, CONST void* transform, UITextureHandle tex) PURE;

	VIRTUAL void METHOD(ReleaseGeometry)(UIGeometryHandle geo) PURE;

	VIRTUAL UITextureHandle METHOD(CreateTexture)(
		CONST void* pixel_data, UInt32 w, UInt32 h) PURE;

	VIRTUAL void METHOD(ReleaseTexture)(UITextureHandle tex) PURE;

	// -----------------------------------------------------------------------
	// Scissor / clipping
	// -----------------------------------------------------------------------

	/// Enable or disable scissor testing.
	VIRTUAL void METHOD(EnableScissor)(bool enable) PURE;

	/// Set the scissor rectangle (in window coordinates: origin top-left, Y-down).
	/// The concrete implementation translates to Vulkan's Y-up convention.
	VIRTUAL void METHOD(SetScissor)(Int x, Int y, UInt32 w, UInt32 h) PURE;

	// -----------------------------------------------------------------------
	// Optional advanced features (default empty implementations)
	// -----------------------------------------------------------------------

	/// Enable stencil-based clip mask. When enabled, subsequent DrawGeometry
	/// calls are tested against the current stencil mask.
	VIRTUAL void METHOD(EnableClipMask)(bool enable) {}

	/// Render geometry into the stencil buffer to define a clip mask region.
	/// @param operation  Set (replace), SetInverse, Intersect
	/// @param geo_handle Geometry defining the mask shape
	/// @param transform  float[16] transform matrix (may be nullptr)
	VIRTUAL void METHOD(RenderToClipMask)(Int operation, UIGeometryHandle geo, CONST void* transform) {}

	VIRTUAL void METHOD(SetTransform)(CONST void* transform) {}

	VIRTUAL Int METHOD(PushLayer)() { return -1; }
	VIRTUAL void METHOD(PopLayer)() {}

	VIRTUAL void METHOD(CompositeLayers)(Int src_layer, Int dst_layer,
		Int blend_mode, CONST UIFilterHandle* filters, UInt32 num_filters) {}

	VIRTUAL UITextureHandle METHOD(SaveLayerAsTexture)() { return {}; }

	VIRTUAL UIFilterHandle METHOD(CompileFilter)(CONST char* name, CONST char* params) { return {}; }

	VIRTUAL void METHOD(ReleaseFilter)(UIFilterHandle filter) {}

	/// Returns true if offscreen rendering is required this frame
	/// (clip masks, layers, or transforms are active).
	VIRTUAL bool METHOD(NeedsOffscreen)() CONST { return false; }

	/// Returns the current viewport height used for coordinate flipping.
	VIRTUAL UInt32 METHOD(GetViewportHeight)() CONST PURE;

protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_UIRENDERER_
