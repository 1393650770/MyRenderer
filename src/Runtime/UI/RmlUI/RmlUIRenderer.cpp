#include "RmlUIRenderer.h"

#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"

#include <fstream>
#include <iostream>
#include <cstring>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

// =========================================================================
// Helper: Read SPIR-V binary from file
// =========================================================================
static Vector<UInt32> ReadShaderFile(CONST String& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "[RmlUIRenderer] Failed to open shader: " << filename << std::endl;
		return {};
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));
	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();
	return buffer;
}

// =========================================================================
// Lifecycle
// =========================================================================
RmlUIRenderer::~RmlUIRenderer()
{
	Shutdown();
}

void RmlUIRenderer::Initialize(RHI::CommandList* cmd_list, ENUM_TEXTURE_FORMAT rt_format)
{
	// Initialize transform to identity
	for (int i = 0; i < 16; i++)
		m_transform[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	m_translation[0] = 0.0f;
	m_translation[1] = 0.0f;

	CreateShaders(cmd_list);
	CreatePSOs(rt_format);

	std::cout << "[RmlUIRenderer] Initialized with " << m_geometries.size()
		<< " geometries, " << m_textures.size() << " textures." << std::endl;
}

void RmlUIRenderer::Shutdown()
{
	m_geometries.clear();
	m_textures.clear();

	delete m_srb_untextured;
	m_srb_untextured = nullptr;

	DestroyPSOs();
	DestroyShaders();
}

// =========================================================================
// BeginFrame / EndFrame
// =========================================================================
void RmlUIRenderer::BeginFrame(RHI::CommandList* cmd)
{
	m_current_cmd = cmd;
	m_stats = {};

	// Reset per-frame state
	EnableScissor(false);
	m_clip_mask_enabled = false;
}

void RmlUIRenderer::EndFrame(RHI::CommandList* cmd)
{
	m_current_cmd = nullptr;
}

// =========================================================================
// Geometry
// =========================================================================
UInt32 RmlUIRenderer::CompileGeometry(
	CONST void* vertices, UInt32 vtx_count, UInt32 vtx_stride,
	CONST void* indices, UInt32 idx_count, bool idx32)
{
	if (!g_render_rhi) return 0;

	UInt32 handle = m_next_geo_handle++;
	GeoSlot slot;

	// Create vertex buffer (Dynamic = host-visible persistent mapping)
	{
		RHI::BufferDesc desc;
		desc.size = vtx_count * vtx_stride;
		desc.stride = vtx_stride;
		desc.type = ENUM_BUFFER_TYPE::Vertex | ENUM_BUFFER_TYPE::Dynamic;
		slot.vb = g_render_rhi->CreateBuffer(desc);
		if (slot.vb)
		{
			void* mapped = slot.vb->Map(ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::Discard);
			if (mapped)
			{
				memcpy(mapped, vertices, desc.size);
				slot.vb->Unmap();
			}
		}
		slot.vtx_count = vtx_count;
	}

	// Create index buffer
	if (indices && idx_count > 0)
	{
		UInt32 idx_size = idx32 ? 4 : 2;
		RHI::BufferDesc desc;
		desc.size = idx_count * idx_size;
		desc.stride = idx_size;
		desc.type = ENUM_BUFFER_TYPE::Index | ENUM_BUFFER_TYPE::Dynamic;
		slot.ib = g_render_rhi->CreateBuffer(desc);
		if (slot.ib)
		{
			void* mapped = slot.ib->Map(ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::Discard);
			if (mapped)
			{
				memcpy(mapped, indices, desc.size);
				slot.ib->Unmap();
			}
		}
		slot.idx_count = idx_count;
	}

	m_geometries[handle] = slot;
	m_stats.geometry_compiled++;
	return handle;
}

void RmlUIRenderer::DrawGeometry(UInt32 geo_handle, CONST void* transform, UInt32 tex_handle)
{
	if (!m_current_cmd) return;

	auto it = m_geometries.find(geo_handle);
	if (it == m_geometries.end()) return;

	CONST GeoSlot& geo = it->second;
	if (!geo.vb) return;

	// Update transform if provided
	if (transform)
	{
		memcpy(m_transform, transform, sizeof(float) * 16);
	}

	// Select PSO based on current state
	bool has_texture = (tex_handle != 0);
	RHI::RenderPipelineState* pso = SelectPSO(has_texture);
	if (!pso) return;

	m_current_cmd->SetGraphicsPipeline(pso);

	// Bind SRB (textured or untextured)
	if (has_texture)
	{
		auto tex_it = m_textures.find(tex_handle);
		if (tex_it != m_textures.end() && tex_it->second.srb)
		{
			// Static SRB — texture was bound at CreateTexture time
			m_current_cmd->SetShaderResourceBinding(tex_it->second.srb);
		}
	}
	else
	{
		if (m_srb_untextured)
		{
			m_current_cmd->SetShaderResourceBinding(m_srb_untextured);
		}
	}

	// Push constants
	FlushPushConstants();

	// Vertex buffer
	m_current_cmd->SetVertexBuffer(geo.vb, 0, 0, 0); // stride=0: the pipeline knows the stride from vertex layout

	// Index buffer
	if (geo.ib && geo.idx_count > 0)
	{
		m_current_cmd->SetIndexBuffer(geo.ib, 0, true); // 32-bit indices
		m_current_cmd->DrawIndexed(geo.idx_count, 1, 0, 0, 0);
	}
	else
	{
		RHI::DrawAttribute da;
		da.vertexCount = geo.vtx_count;
		da.instanceCount = 1;
		m_current_cmd->Draw(da);
	}

	m_stats.draw_calls++;
}

void RmlUIRenderer::ReleaseGeometry(UInt32 geo_handle)
{
	auto it = m_geometries.find(geo_handle);
	if (it == m_geometries.end()) return;

	delete it->second.vb;
	delete it->second.ib;
	m_geometries.erase(it);
}

// =========================================================================
// Texture
// =========================================================================
UInt32 RmlUIRenderer::CreateTexture(CONST void* pixel_data, UInt32 w, UInt32 h)
{
	if (!g_render_rhi || !pixel_data) return 0;

	UInt32 handle = m_next_tex_handle++;
	TexSlot slot;

	RHI::TextureDesc desc;
	desc.width = w;
	desc.height = h;
	desc.mip_level = 1;
	desc.layer_count = 1;
	desc.format = ENUM_TEXTURE_FORMAT::RGBA8;
	desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
	desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
	desc.resource_state = ENUM_RESOURCE_STATE::ShaderResource;

	slot.texture = g_render_rhi->CreateTexture(desc);
	if (slot.texture)
	{
		// Upload pixel data
		RHI::TextureDataPayload payload;
		payload.data.resize(w * h * 4);
		memcpy(payload.data.data(), pixel_data, w * h * 4);
		payload.format = ENUM_TEXTURE_FORMAT::RGBA8;
		payload.width = w;
		payload.height = h;
		slot.texture->UpdateTextureData(payload);

		// Create static SRB for this texture
		// Use the textured PSO as template
		if (m_pso_textured)
		{
			m_pso_textured->CreateShaderResourceBinding(slot.srb, true);
			if (slot.srb)
			{
				slot.srb->SetResource("tex", slot.texture);
			}
		}
	}

	m_textures[handle] = slot;
	m_stats.textures_created++;
	return handle;
}

void RmlUIRenderer::ReleaseTexture(UInt32 tex_handle)
{
	auto it = m_textures.find(tex_handle);
	if (it == m_textures.end()) return;

	delete it->second.srb;
	delete it->second.texture; // TODO: verify ownership — texture deletion may crash if pool-managed
	m_textures.erase(it);
}

// =========================================================================
// Scissor
// =========================================================================
void RmlUIRenderer::EnableScissor(bool enable)
{
	m_scissor_enabled = enable;
	if (m_current_cmd)
	{
		auto* vk_cmd = STATIC_CAST(m_current_cmd, RHI::Vulkan::VK_CommandBuffer);
		if (vk_cmd)
		{
			vk_cmd->SetScissorEnable(enable);
		}
	}
}

void RmlUIRenderer::SetScissor(Int32 x, Int32 y, UInt32 w, UInt32 h)
{
	m_scissor_x = x;
	m_scissor_y = y;
	m_scissor_w = w;
	m_scissor_h = h;

	if (m_current_cmd)
	{
		auto* vk_cmd = STATIC_CAST(m_current_cmd, RHI::Vulkan::VK_CommandBuffer);
		if (vk_cmd)
		{
			// Windows (Y-down) → Vulkan (Y-up) flip
			Int32 vk_y = static_cast<Int32>(m_viewport_h) - y - static_cast<Int32>(h);
			if (vk_y < 0) vk_y = 0;
			vk_cmd->SetScissor(x, vk_y, w, h);
		}
	}
}

// =========================================================================
// Clip mask (stencil)
// =========================================================================
void RmlUIRenderer::EnableClipMask(bool enable)
{
	m_clip_mask_enabled = enable;
	m_stats.used_clip_mask = m_stats.used_clip_mask || enable;
}

void RmlUIRenderer::RenderToClipMask(Int32 operation, UInt32 geo_handle, CONST void* transform)
{
	// operation: 0=Set, 1=SetInverse, 2=Intersect
	if (!m_current_cmd) return;

	RHI::RenderPipelineState* pso = nullptr;
	if (operation == 0)
		pso = m_pso_stencil_set;      // ALWAYS REPLACE
	else if (operation == 2)
		pso = m_pso_stencil_intersect; // EQUAL REPLACE
	else
		pso = m_pso_stencil_set;      // fallback

	if (!pso) return;
	m_current_cmd->SetGraphicsPipeline(pso);

	// Draw geometry with color write mask = 0 (stencil only)
	DrawGeometry(geo_handle, transform, 0);
}

// =========================================================================
// Transform
// =========================================================================
void RmlUIRenderer::SetTransform(CONST void* transform)
{
	if (transform)
	{
		memcpy(m_transform, transform, sizeof(float) * 16);
		m_stats.used_transform = true;
	}
	else
	{
		// Identity
		for (int i = 0; i < 16; i++)
			m_transform[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	}
}

bool RmlUIRenderer::NeedsOffscreen() CONST
{
	return m_clip_mask_enabled || m_stats.used_layers || m_stats.used_transform;
}

UInt32 RmlUIRenderer::GetViewportHeight() CONST
{
	return m_viewport_h;
}

// =========================================================================
// PSO selection
// =========================================================================
RHI::RenderPipelineState* RmlUIRenderer::SelectPSO(bool has_texture) CONST
{
	if (m_clip_mask_enabled)
	{
		return has_texture ? m_pso_textured_stencil : m_pso_untextured_stencil;
	}
	return has_texture ? m_pso_textured : m_pso_untextured;
}

// =========================================================================
// Push constants
// =========================================================================
void RmlUIRenderer::FlushPushConstants()
{
	if (!m_current_cmd) return;

	// Layout: mat4(64 bytes) + vec2(8 bytes) + padding(8 to align to 16)
	struct {
		float tf[16];    // 64 bytes
		float tl[2];     // 8 bytes
		float _pad[2];   // 8 bytes padding
	} pc;

	memcpy(pc.tf, m_transform, sizeof(pc.tf));
	pc.tl[0] = m_translation[0];
	pc.tl[1] = m_translation[1];
	pc._pad[0] = 0.0f;
	pc._pad[1] = 0.0f;

	m_current_cmd->SetPushConstants(0, sizeof(pc), &pc);
}

// =========================================================================
// Shader creation
// =========================================================================
void RmlUIRenderer::CreateShaders(RHI::CommandList* cmd_list)
{
	RHI::ShaderDesc sd;
	RHI::ShaderDataPayload sp;

	// Vertex shader
	sd.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
	sd.shader_name = "RmlUIVS";
	sd.entry_name = "main";
	sp.data = ReadShaderFile("Shader/rml_ui.vert.spv");
	if (!sp.data.empty())
		m_vs = g_render_rhi->CreateShader(sd, sp);

	// Fragment shader (textured)
	sd.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
	sd.shader_name = "RmlUIFS";
	sp.data = ReadShaderFile("Shader/rml_ui.frag.spv");
	if (!sp.data.empty())
		m_fs_textured = g_render_rhi->CreateShader(sd, sp);

	// Fragment shader (composite)
	sd.shader_name = "RmlUICompositeFS";
	sp.data = ReadShaderFile("Shader/rml_ui_composite.frag.spv");
	if (!sp.data.empty())
		m_fs_composite = g_render_rhi->CreateShader(sd, sp);

	std::cout << "[RmlUIRenderer] Shaders loaded: vs=" << (m_vs != nullptr)
		<< " fs_tex=" << (m_fs_textured != nullptr)
		<< " fs_comp=" << (m_fs_composite != nullptr) << std::endl;
}

void RmlUIRenderer::DestroyShaders()
{
	delete m_vs; m_vs = nullptr;
	delete m_fs_textured; m_fs_textured = nullptr;
	delete m_fs_composite; m_fs_composite = nullptr;
}

// =========================================================================
// PSO creation
// =========================================================================
void RmlUIRenderer::CreatePSOs(ENUM_TEXTURE_FORMAT rt_format)
{
	if (!g_render_rhi || !m_vs || !m_fs_textured) return;

	auto MakeBaseDesc = [&]() {
		RHI::RenderGraphiPipelineStateDesc desc;
		desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = m_vs;
		desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = m_fs_textured;
		desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		desc.raster_state.cull_mode = ENUM_RASTER_CULLMODE::ENUM_NONE;
		desc.raster_state.fill_mode = ENUM_RASTER_FILLMODE::ENUM_SOLID;
		desc.raster_state.front_counter_clockwise = true; // RmlUI uses GL winding (clockwise = front)
		desc.raster_state.scissor_enable = true;
		desc.raster_state.sample_count = 1;

		// Premultiplied alpha blend
		desc.blend_state.render_targets.resize(1);
		desc.blend_state.render_targets[0].blend_enable = true;
		desc.blend_state.render_targets[0].src_color = ENUM_BLEND_FACTOR::ENUM_SRC_ALPHA;
		desc.blend_state.render_targets[0].dst_color = ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_SRC_ALPHA;
		desc.blend_state.render_targets[0].src_alpha = ENUM_BLEND_FACTOR::ENUM_ONE;
		desc.blend_state.render_targets[0].dst_alpha = ENUM_BLEND_FACTOR::ENUM_ONE_MINUS_SRC_ALPHA;
		desc.blend_state.render_targets[0].op_color = ENUM_BLEND_EQUATION::ENUM_ADD;
		desc.blend_state.render_targets[0].op_alpha = ENUM_BLEND_EQUATION::ENUM_ADD;
		desc.blend_state.render_targets[0].write_mask = ENUM_COLOR_MASK::ENUM_All;

		// Depth/stencil: disabled by default
		desc.depth_stencil_state.depth_test_enable = false;
		desc.depth_stencil_state.depth_write_enable = false;
		desc.depth_stencil_state.stencil_test_enable = false;

		// Vertex input: Rml::Vertex = pos(vec2) + color(ubyte4) + uv(vec2) = 20 bytes
		RHI::VertexInputLayout pos_layout;
		pos_layout.binding = 0;
		pos_layout.location = 0;
		pos_layout.attribute_format = ENUM_TEXTURE_FORMAT::RG32F; // float2
		pos_layout.offset = 0;
		desc.vertex_input_layout.push_back(pos_layout);

		RHI::VertexInputLayout color_layout;
		color_layout.binding = 0;
		color_layout.location = 1;
		color_layout.attribute_format = ENUM_TEXTURE_FORMAT::RGBA8; // ubyte4 normalized
		color_layout.offset = 8;
		desc.vertex_input_layout.push_back(color_layout);

		RHI::VertexInputLayout uv_layout;
		uv_layout.binding = 0;
		uv_layout.location = 2;
		uv_layout.attribute_format = ENUM_TEXTURE_FORMAT::RG32F; // float2
		uv_layout.offset = 12;
		desc.vertex_input_layout.push_back(uv_layout);

		return desc;
	};

	// === PSO 1: Textured ===
	{
		auto desc = MakeBaseDesc();
		m_pso_textured = g_render_rhi->CreateRenderPipelineState(desc);
		if (m_pso_textured)
		{
			m_pso_textured->CreateShaderResourceBinding(m_srb_untextured, true);
		}
	}

	// === PSO 2: Untextured (same PSO, fragment shader handles missing texture) ===
	// Actually: RmlUI always uses texture; untextured elements have a white 1x1 texture.
	// We share the same PSO as textured for now.
	m_pso_untextured = m_pso_textured; // alias

	// === PSO 3: Stencil Set (color write mask = 0, stencil REPLACE) ===
	{
		auto desc = MakeBaseDesc();
		desc.blend_state.render_targets[0].write_mask = ENUM_COLOR_MASK(0); // no color output
		desc.depth_stencil_state.stencil_test_enable = true;
		desc.depth_stencil_state.stencil_write_mask = 0xFF;
		desc.depth_stencil_state.stencil_ref = 1;
		desc.depth_stencil_state.front_face_stencil.func = ENUM_STENCIL_FUNCTION::ENUM_ALWAYS;
		desc.depth_stencil_state.front_face_stencil.pass_op = ENUM_STENCIL_OPERATIOON::ENUM_REPLACE;
		desc.depth_stencil_state.front_face_stencil.fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.front_face_stencil.depth_fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.back_face_stencil = desc.depth_stencil_state.front_face_stencil;
		m_pso_stencil_set = g_render_rhi->CreateRenderPipelineState(desc);
	}

	// === PSO 4: Stencil Intersect (stencil test EQUAL, write REPLACE with incremented ref) ===
	{
		auto desc = MakeBaseDesc();
		desc.blend_state.render_targets[0].write_mask = ENUM_COLOR_MASK(0);
		desc.depth_stencil_state.stencil_test_enable = true;
		desc.depth_stencil_state.stencil_read_mask = 0xFF;
		desc.depth_stencil_state.stencil_write_mask = 0xFF;
		desc.depth_stencil_state.front_face_stencil.func = ENUM_STENCIL_FUNCTION::ENUM_EQUAL;
		desc.depth_stencil_state.front_face_stencil.pass_op = ENUM_STENCIL_OPERATIOON::ENUM_REPLACE;
		desc.depth_stencil_state.front_face_stencil.fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.front_face_stencil.depth_fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.back_face_stencil = desc.depth_stencil_state.front_face_stencil;
		m_pso_stencil_intersect = g_render_rhi->CreateRenderPipelineState(desc);
	}

	// === PSO 5: Textured + Stencil Test (read stencil, test EQUAL) ===
	{
		auto desc = MakeBaseDesc();
		desc.depth_stencil_state.stencil_test_enable = true;
		desc.depth_stencil_state.stencil_read_mask = 0xFF;
		desc.depth_stencil_state.stencil_write_mask = 0; // read-only
		desc.depth_stencil_state.front_face_stencil.func = ENUM_STENCIL_FUNCTION::ENUM_EQUAL;
		desc.depth_stencil_state.front_face_stencil.pass_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.front_face_stencil.fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.front_face_stencil.depth_fail_op = ENUM_STENCIL_OPERATIOON::ENUM_KEEP;
		desc.depth_stencil_state.back_face_stencil = desc.depth_stencil_state.front_face_stencil;
		m_pso_textured_stencil = g_render_rhi->CreateRenderPipelineState(desc);
	}

	// === PSO 6: Untextured + Stencil Test ===
	m_pso_untextured_stencil = m_pso_textured_stencil; // alias (same PSO for v1)

	std::cout << "[RmlUIRenderer] PSOs created: tex=" << (m_pso_textured != nullptr)
		<< " stencil_set=" << (m_pso_stencil_set != nullptr)
		<< " stencil_isect=" << (m_pso_stencil_intersect != nullptr)
		<< " stencil_test=" << (m_pso_textured_stencil != nullptr) << std::endl;
}

void RmlUIRenderer::DestroyPSOs()
{
	// PSOs are owned by VK_PipelineStateManager — do NOT delete them directly.
	// The pointers here are non-owning references.
	m_pso_textured = nullptr;
	m_pso_untextured = nullptr;
	m_pso_stencil_set = nullptr;
	m_pso_stencil_intersect = nullptr;
	m_pso_textured_stencil = nullptr;
	m_pso_untextured_stencil = nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
