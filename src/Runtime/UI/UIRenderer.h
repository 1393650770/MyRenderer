
#pragma once
#ifndef _UIRENDERER_
#define _UIRENDERER_

#include "Core/ConstDefine.h"

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
	VIRTUAL UInt32 METHOD(CompileGeometry)(
		CONST void* vertices, UInt32 vtx_count, UInt32 vtx_stride,
		CONST void* indices,  UInt32 idx_count, bool idx32) PURE;

	/// Draw previously compiled geometry with an optional texture and transform.
	/// @param geo_handle      Handle from CompileGeometry()
	/// @param transform       float[16] or float[20] transform matrix (may be nullptr for identity)
	/// @param tex_handle      Texture handle from CreateTexture(), or 0 for untextured
	VIRTUAL void METHOD(DrawGeometry)(
		UInt32 geo_handle, CONST void* transform, UInt32 tex_handle) PURE;

	/// Release a compiled geometry handle and its GPU resources.
	VIRTUAL void METHOD(ReleaseGeometry)(UInt32 geo_handle) PURE;

	// -----------------------------------------------------------------------
	// Texture management
	// -----------------------------------------------------------------------

	/// Create a 2D texture from raw pixel data (RGBA8 premultiplied alpha).
	/// @param pixel_data  RGBA8 pixel buffer (w * h * 4 bytes)
	/// @param w, h        Dimensions in pixels
	/// @return Opaque texture handle (>0), or 0 on failure
	VIRTUAL UInt32 METHOD(CreateTexture)(
		CONST void* pixel_data, UInt32 w, UInt32 h) PURE;

	/// Release a texture handle and its GPU resources.
	VIRTUAL void METHOD(ReleaseTexture)(UInt32 tex_handle) PURE;

	// -----------------------------------------------------------------------
	// Scissor / clipping
	// -----------------------------------------------------------------------

	/// Enable or disable scissor testing.
	VIRTUAL void METHOD(EnableScissor)(bool enable) PURE;

	/// Set the scissor rectangle (in window coordinates: origin top-left, Y-down).
	/// The concrete implementation translates to Vulkan's Y-up convention.
	VIRTUAL void METHOD(SetScissor)(Int32 x, Int32 y, UInt32 w, UInt32 h) PURE;

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
	VIRTUAL void METHOD(RenderToClipMask)(Int32 operation, UInt32 geo_handle, CONST void* transform) {}

	/// Apply a 2D transform to subsequent geometry (e.g. rotation, scale).
	/// @param transform float[16] column-major matrix, or nullptr to use identity
	VIRTUAL void METHOD(SetTransform)(CONST void* transform) {}

	/// Push a new render target onto the layer stack. Creates an offscreen
	/// color+DS texture pair at viewport resolution, cleared to transparent black.
	/// @return New layer index (0 = backbuffer, 1+ = offscreen layers)
	VIRTUAL Int32 METHOD(PushLayer)() { return -1; }

	/// Pop the top render target from the layer stack, restoring the previous target.
	VIRTUAL void METHOD(PopLayer)() {}

	/// Composite a source layer onto a destination layer with optional filters.
	/// @param src_layer  Source layer index
	/// @param dst_layer  Destination layer index (0 = backbuffer)
	/// @param blend_mode 0=alpha blend, 1=replace
	/// @param filters    Array of compiled filter handles (may be nullptr)
	/// @param num_filters Number of filter handles
	VIRTUAL void METHOD(CompositeLayers)(Int32 src_layer, Int32 dst_layer,
		Int32 blend_mode, CONST UInt32* filters, UInt32 num_filters) {}

	/// Save the current layer as a texture for later use (e.g. box-shadow).
	/// @return Texture handle
	VIRTUAL UInt32 METHOD(SaveLayerAsTexture)() { return 0; }

	/// Compile a named filter (e.g. "blur") with parameters.
	/// @param name  Filter name (e.g. "blur")
	/// @param params JSON-like key-value pairs, nullptr-terminated
	/// @return Filter handle, or 0 if unsupported
	VIRTUAL UInt32 METHOD(CompileFilter)(CONST char* name, CONST char* params) { return 0; }

	/// Release a compiled filter.
	VIRTUAL void METHOD(ReleaseFilter)(UInt32 filter_handle) {}

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
