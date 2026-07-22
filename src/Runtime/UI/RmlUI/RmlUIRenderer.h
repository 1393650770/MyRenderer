#pragma once
#ifndef _RMLUIRENDERER_
#define _RMLUIRENDERER_

#include "Core/ConstDefine.h"
#include "UI/UIRenderer.h"
#include "UI/UIDrawBuffer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Buffer;
class Texture;
class Shader;
class RenderPipelineState;
class ShaderResourceBinding;
class CommandList;
MYRENDERER_END_NAMESPACE
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

/**
 * Concrete UIRenderer implementation for RmlUI on Vulkan.
 *
 * Owns the GPU resources needed to draw RmlUI geometry:
 * - 6 PSOs (textured, untextured, 2 stencil variants, 2 stencil-test variants)
 * - Vertex/index buffer ring allocator for compiled geometry
 * - Texture cache (SRB per texture)
 * - Push constant management
 * - Scissor and stencil state tracking
 *
 * Thread safety: All methods must be called from the render thread
 * (inside the RDG pass execute lambda).
 */
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RmlUIRenderer, public UIRenderer)

#pragma region METHOD
public:
	RmlUIRenderer() MYDEFAULT;
	VIRTUAL ~RmlUIRenderer() MYDEFAULT;

	/// Initialize Vulkan resources (shaders, PSOs). Must be called after RHI is ready.
	void METHOD(Initialize)(RHI::CommandList* cmd_list, ENUM_TEXTURE_FORMAT rt_format);

	/// Release all Vulkan resources.
	void METHOD(Shutdown)();

	// === UIRenderer interface ===

	VIRTUAL void METHOD(BeginFrame)(RHI::CommandList* cmd) OVERRIDE;
	VIRTUAL void METHOD(EndFrame)(RHI::CommandList* cmd) OVERRIDE;

	VIRTUAL UInt32 METHOD(CompileGeometry)(
		CONST void* vertices, UInt32 vtx_count, UInt32 vtx_stride,
		CONST void* indices, UInt32 idx_count, bool idx32) OVERRIDE;

	VIRTUAL void METHOD(DrawGeometry)(
		UInt32 geo_handle, CONST void* transform, UInt32 tex_handle) OVERRIDE;

	VIRTUAL void METHOD(ReleaseGeometry)(UInt32 geo_handle) OVERRIDE;

	VIRTUAL UInt32 METHOD(CreateTexture)(
		CONST void* pixel_data, UInt32 w, UInt32 h) OVERRIDE;

	VIRTUAL void METHOD(ReleaseTexture)(UInt32 tex_handle) OVERRIDE;

	VIRTUAL void METHOD(EnableScissor)(bool enable) OVERRIDE;
	VIRTUAL void METHOD(SetScissor)(Int32 x, Int32 y, UInt32 w, UInt32 h) OVERRIDE;

	// Optional overrides
	VIRTUAL void METHOD(EnableClipMask)(bool enable) OVERRIDE;
	VIRTUAL void METHOD(RenderToClipMask)(Int32 operation, UInt32 geo_handle, CONST void* transform) OVERRIDE;
	VIRTUAL void METHOD(SetTransform)(CONST void* transform) OVERRIDE;
	VIRTUAL bool METHOD(NeedsOffscreen)() CONST OVERRIDE;
	VIRTUAL UInt32 METHOD(GetViewportHeight)() CONST OVERRIDE;

protected:
private:
	// Shaders
	RHI::Shader* m_vs = nullptr;
	RHI::Shader* m_fs_textured = nullptr;
	RHI::Shader* m_fs_composite = nullptr;

	// Pipeline states (6 total)
	RHI::RenderPipelineState* m_pso_textured = nullptr;
	RHI::RenderPipelineState* m_pso_untextured = nullptr;
	RHI::RenderPipelineState* m_pso_stencil_set = nullptr;
	RHI::RenderPipelineState* m_pso_stencil_intersect = nullptr;
	RHI::RenderPipelineState* m_pso_textured_stencil = nullptr;
	RHI::RenderPipelineState* m_pso_untextured_stencil = nullptr;

	// SRB for untextured draw (no texture binding needed)
	RHI::ShaderResourceBinding* m_srb_untextured = nullptr;

	// Current command list (set by BeginFrame)
	RHI::CommandList* m_current_cmd = nullptr;

	// Viewport height for scissor Y-flip
	UInt32 m_viewport_h = 0;

	// Geometry handle → buffer mapping
	struct GeoSlot {
		RHI::Buffer* vb = nullptr;
		RHI::Buffer* ib = nullptr;
		UInt32 vtx_count = 0;
		UInt32 idx_count = 0;
	};
	Map<UInt32, GeoSlot> m_geometries;
	UInt32 m_next_geo_handle = 1;

	// Texture handle → texture mapping
	struct TexSlot {
		RHI::Texture* texture = nullptr;
		RHI::ShaderResourceBinding* srb = nullptr;
	};
	Map<UInt32, TexSlot> m_textures;
	UInt32 m_next_tex_handle = 1;

	// Push constant layout (matches rml_ui.vert)
	static constexpr UInt32 c_push_constant_size = 80; // mat4(64) + vec2(8) + padding(8)
	float m_transform[16];  // current transform matrix (identity by default)
	float m_translation[2]; // current translation

	// Scissor state
	bool m_scissor_enabled = false;
	Int32 m_scissor_x = 0, m_scissor_y = 0;
	UInt32 m_scissor_w = 0, m_scissor_h = 0;

	// Clip mask state
	bool m_clip_mask_enabled = false;
	Int32 m_stencil_ref = 1;

	// Layer stack (0 = backbuffer)
	Int32 m_current_layer = 0;

	// Stats
	UIDrawStats m_stats;

	// Internal helpers
	void METHOD(CreatePSOs)(ENUM_TEXTURE_FORMAT rt_format);
	void METHOD(CreateShaders)(RHI::CommandList* cmd_list);
	void METHOD(DestroyPSOs)();
	void METHOD(DestroyShaders)();
	void METHOD(FlushPushConstants)();

	/// Selects the appropriate PSO based on current state
	RHI::RenderPipelineState* METHOD(SelectPSO)(bool has_texture) CONST;
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_RMLUIRENDERER_
