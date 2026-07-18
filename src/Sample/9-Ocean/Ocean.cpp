// Ocean: Gerstner-wave mesh with standard vertex/index buffers + DrawIndexed.
// Grid generated once on GPU via compute shader.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <cstring>
#include "Application/Window.h"
#include "Render/RenderInterface.h"
#include "Render/Core/RenderGraph.h"
#include "RHI/RenderRource.h"
#include "Render/Core/RenderGraphPass.h"
#include "RHI/RenderPass.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"
#include "RHI/Vulkan/VK_Viewport.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderShader.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/RenderBuffer.h"
using namespace MXRender::RHI;
using namespace MXRender::Render;
using namespace MXRender::Application;
using namespace MXRender::RHI::Vulkan;
using namespace MXRender;

static CONST UInt32 GRID_DIM = 512;
static CONST UInt32 NUM_VERTS = (GRID_DIM + 1) * (GRID_DIM + 1);
static CONST UInt32 NUM_INDICES = GRID_DIM * GRID_DIM * 6;
static CONST Float32 MESH_SIZE = 204.8f;
static CONST Float32 FOG_DENSITY = 0.0045f;

static Float32 g_scroll = 0.0f;
static void ScrollCB(GLFWwindow*, Float64, Float64 y) { g_scroll += (Float32)y; }

Vector<UInt32> RS(CONST String& fn) {
	std::ifstream f(fn, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!f.is_open(), "fail shader"); size_t sz = (size_t)f.tellg();
	Vector<UInt32> b(sz / sizeof(UInt32)); f.seekg(0); f.read((char*)b.data(), sz);
	return std::move(b);
}
static Shader* LF(ENUM_SHADER_STAGE st, CONST String& fn) {
	ShaderDesc d; d.shader_type = st; d.entry_name = "main"; d.shader_name = fn;
	ShaderDataPayload p; p.data = RS(fn); return RHICreateShader(d, p);
}
static RenderPipelineState* CreateCompPSO(Shader* cs) {
	RenderGraphiPipelineStateDesc d{};
	d.shaders[ENUM_SHADER_STAGE::Shader_Compute] = cs;
	d.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
	d.raster_state.sample_count = 1;
	return RHICreateRenderPipelineState(d);
}
static void UF(Buffer* b, CONST Float32* d, UInt32 n) {
	void* p = RHIMapBuffer(b, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(p, d, n * sizeof(Float32)); RHIUnmapBuffer(b);
}

struct PD : public RenderGraphPassDataBase {
	RenderPipelineState* pso = nullptr; ShaderResourceBinding* srb = nullptr;
	VIRTUAL ~PD() MYDEFAULT;
	VIRTUAL void Release() OVERRIDE { delete pso; delete srb; }
};

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RT, public MXRender::RenderInterface)
public:
	RT(Window* w) : window(w) {} RT() MYDEFAULT; VIRTUAL ~RT() MYDEFAULT;
	VIRTUAL void OnInit_Logic(Application::Window*) OVERRIDE FINAL;
	VIRTUAL void OnShutdown_Logic() OVERRIDE FINAL;
	VIRTUAL void OnUpdate(float) OVERRIDE FINAL;
	VIRTUAL void OnRender() OVERRIDE FINAL;
	Window* GetWindow() { return window; }
protected:
	Window* window = nullptr;
	RHI::Buffer *op_buf = nullptr, *vtx_buf = nullptr, *idx_buf = nullptr;
	Float32 time_sec = 0; Int dm = 0; Bool ds = false, kp[7] = {};
	Float32 cyaw = 0.7f, cpitch = 0.42f, cdist = 60; Bool drag = false; Float64 lcx = 0, lcy = 0;
	glm::vec3 ceye = glm::vec3(0, 25, 50), sun = glm::normalize(glm::vec3(-0.55f, 0.32f, -0.60f));
	glm::mat4 vp = glm::mat4(1);
MYRENDERER_END_CLASS

void RT::OnInit_Logic(Application::Window* in_window) {
	window = in_window; std::cout << "Hello Ocean (Indexed)" << std::endl;
	glfwSetScrollCallback(window->GetWindow(), ScrollCB);

	RHI::CommandList* cl = RHIGetImmediateCommandList();
	RHI::Texture* bb = window->GetViewport()->GetCurrentBackBufferRTV();
	RHI::Texture* ds = window->GetViewport()->GetCurrentBackBufferDSV();

	// params buffer
	BufferDesc od; od.type = ENUM_BUFFER_TYPE::Storage | ENUM_BUFFER_TYPE::Dynamic;
	od.size = od.stride = 64 * sizeof(Float32); op_buf = RHICreateBuffer(od);
	Float32 z[64] = {}; UF(op_buf, z, 64);

	// CPU grid generation (Vulkan compute init dispatch needs an active CB,
	// so generate data on CPU and upload via Map/Unmap)
	BufferDesc vd{}; vd.type = ENUM_BUFFER_TYPE::Storage | ENUM_BUFFER_TYPE::Dynamic | ENUM_BUFFER_TYPE::Vertex;
	vd.size = NUM_VERTS * 4 * sizeof(Float32); vd.stride = 4 * sizeof(Float32);
	vtx_buf = RHICreateBuffer(vd);
	BufferDesc idb{}; idb.type = ENUM_BUFFER_TYPE::Storage | ENUM_BUFFER_TYPE::Dynamic | ENUM_BUFFER_TYPE::Index;
	idb.size = NUM_INDICES * sizeof(UInt32); idb.stride = sizeof(UInt32);
	idx_buf = RHICreateBuffer(idb);

	// CPU vertex data: (x, z, u, v) per grid point
	Vector<Float32> vtx_data(NUM_VERTS * 4);
	for (UInt32 y = 0; y <= GRID_DIM; ++y) {
		for (UInt32 x = 0; x <= GRID_DIM; ++x) {
			UInt32 i = (y * (GRID_DIM + 1) + x) * 4;
			Float32 gx = (Float32)x / GRID_DIM - 0.5f;
			Float32 gy = (Float32)y / GRID_DIM - 0.5f;
			vtx_data[i + 0] = gx * MESH_SIZE;  // world x
			vtx_data[i + 1] = gy * MESH_SIZE;  // world z
			vtx_data[i + 2] = gx + 0.5f;       // u
			vtx_data[i + 3] = 1.0f - (gy + 0.5f); // v
		}
	}
	void* vp = RHIMapBuffer(vtx_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(vp, vtx_data.data(), vtx_data.size() * sizeof(Float32));
	RHIUnmapBuffer(vtx_buf);

	// CPU index data: 6 indices per quad
	Vector<UInt32> idx_data(NUM_INDICES);
	for (UInt32 y = 0; y < GRID_DIM; ++y) {
		for (UInt32 x = 0; x < GRID_DIM; ++x) {
			UInt32 quad = y * GRID_DIM + x;
			UInt32 i0 = y * (GRID_DIM + 1) + x;
			UInt32 i1 = i0 + 1;
			UInt32 i2 = i0 + (GRID_DIM + 1);
			UInt32 i3 = i2 + 1;
			UInt32 base = quad * 6;
			idx_data[base + 0] = i0; idx_data[base + 1] = i1; idx_data[base + 2] = i2;
			idx_data[base + 3] = i1; idx_data[base + 4] = i3; idx_data[base + 5] = i2;
		}
	}
	void* ip = RHIMapBuffer(idx_buf, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
	std::memcpy(ip, idx_data.data(), idx_data.size() * sizeof(UInt32));
	RHIUnmapBuffer(idx_buf);

	auto* rt = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>("RT", bb->GetTextureDesc(), bb);
	RenderGraphResource<RHI::TextureDesc, RHI::Texture>* dv = nullptr;
	if (ds) dv = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>("DS", ds->GetTextureDesc(), ds);

	// Sky pass (CLEAR, 0 bindings)
	auto* p1 = graph.AddRenderPass<PD>("Sky", &graph, cl,
	[&](PD& d, RenderGraphPassBuilder& b, CommandList*) {
		b.Write(rt); if (dv) b.Write(dv);
		Shader* vs = LF(ENUM_SHADER_STAGE::Shader_Vertex, "Shader/ocean_sky.vert.spv");
		Shader* ps = LF(ENUM_SHADER_STAGE::Shader_Pixel, "Shader/ocean_sky.frag.spv");
		RenderGraphiPipelineStateDesc pd{}; pd.shaders[0] = vs; pd.shaders[1] = ps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<Texture*> rts = { bb }; pd.render_targets = rts; pd.depth_stencil_view = ds;
		pd.raster_state.sample_count = 1; pd.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pd.blend_state.render_targets.resize(1);
		d.pso = RHICreateRenderPipelineState(pd);
		d.pso->CreateShaderResourceBinding(d.srb, false); d.srb->FlushDescriptorWrites();
		delete vs; delete ps;
	},
	[=](CONST PD& d, CommandList* c) {
		Vector<ClearValue> cc; cc.push_back({ 0.2f, 0.2f, 0.3f, 1.0f });
		Texture* d2 = window->GetViewport()->GetCurrentBackBufferDSV();
		if (d2) cc.push_back({ 1.0f, 0 });
		c->SetRenderTarget({ window->GetViewport()->GetCurrentBackBufferRTV() }, d2, cc, d2 != nullptr);
		c->SetGraphicsPipeline(d.pso); c->SetShaderResourceBinding(d.srb);
		c->Draw(DrawAttribute{ 3, 1, 0, 0 });
	}); p1->SetIsCullable(false);

	// Ocean pass (vertex/index buffer, op_buf only, LOAD, depth test)
	auto* p2 = graph.AddRenderPass<PD>("Ocean", &graph, cl,
	[&](PD& d, RenderGraphPassBuilder& b, CommandList*) {
		b.Write(rt); if (dv) b.Write(dv);
		Shader* vs = LF(ENUM_SHADER_STAGE::Shader_Vertex, "Shader/ocean_surface.vert.spv");
		Shader* ps = LF(ENUM_SHADER_STAGE::Shader_Pixel, "Shader/ocean_surface.frag.spv");
		RenderGraphiPipelineStateDesc pd{}; pd.shaders[0] = vs; pd.shaders[1] = ps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<Texture*> rts = { bb }; pd.render_targets = rts; pd.depth_stencil_view = ds;
		pd.raster_state.sample_count = 1; pd.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pd.depth_stencil_state.depth_test_enable = true;
		pd.depth_stencil_state.depth_write_enable = true;
		pd.depth_stencil_state.depth_func = ENUM_STENCIL_FUNCTION::ENUM_LESS;
		VertexInputLayout vil;
		vil.binding = 0; vil.location = 0;
		vil.attribute_format = ENUM_TEXTURE_FORMAT::RGBA32F;
		vil.offset = 0; vil.input_rate = ENUM_VERTEX_INPUTRATE::PerVertex;
		pd.vertex_input_layout.push_back(vil);
		pd.blend_state.render_targets.resize(1);
		d.pso = RHICreateRenderPipelineState(pd);
		d.pso->CreateShaderResourceBinding(d.srb, false);
		d.srb->SetResource("op", op_buf);
		d.srb->FlushDescriptorWrites();
		delete vs; delete ps;
	},
	[=](CONST PD& d, CommandList* c) {
		Float32 prm[64] = {};
		prm[0] = time_sec; prm[4] = ceye.x; prm[5] = ceye.y; prm[6] = ceye.z;
		prm[8] = sun.x; prm[9] = sun.y; prm[10] = sun.z;
		prm[13] = 1.0f; prm[14] = 0.35f;
		std::memcpy(prm + 16, &vp, 64); prm[48] = FOG_DENSITY;
		prm[58] = (Float32)dm;
		UF(op_buf, prm, 64);

		Vector<ClearValue> nc;
		Texture* d2 = window->GetViewport()->GetCurrentBackBufferDSV();
		c->SetRenderTarget({ window->GetViewport()->GetCurrentBackBufferRTV() }, d2, nc, false);
		c->SetGraphicsPipeline(d.pso); c->SetShaderResourceBinding(d.srb);
		c->SetVertexBuffer(vtx_buf, 0, 4 * sizeof(Float32), 0);
		c->SetIndexBuffer(idx_buf, 0, true);
		c->DrawIndexed(NUM_INDICES, 1, 0, 0, 0);
	}); p2->SetIsCullable(false);

	graph.Compile();
}
void RT::OnShutdown_Logic() { graph.Release(); delete op_buf; delete vtx_buf; delete idx_buf; }
void RT::OnUpdate(float dt) {
	g_scroll = 0; GLFWwindow* w = window->GetWindow(); int ww, wh; glfwGetWindowSize(w, &ww, &wh);
	Float64 cx, cy; glfwGetCursorPos(w, &cx, &cy);
	Bool dn = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (dn && drag) { cyaw += (Float32)(cx - lcx) * 0.005f; cpitch = glm::clamp(cpitch + (Float32)(cy - lcy) * 0.005f, 0.06f, 1.45f); }
	drag = dn; lcx = cx; lcy = cy; cdist = glm::clamp(cdist * powf(0.9f, g_scroll), 8.f, 300.f);
	CONST Int ks[7] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_UP, GLFW_KEY_DOWN };
	Bool kn[7]; for (Int i = 0; i < 7; ++i) kn[i] = glfwGetKey(w, ks[i]) == GLFW_PRESS;
	if (kn[0] && !kp[0]) { dm = 0; ds = false; }
	if (kn[1] && !kp[1]) { dm = 1; ds = false; }
	if (kn[2] && !kp[2]) { dm = 2; ds = false; }
	if (kn[3] && !kp[3]) { dm = 3; ds = false; }
	if (kn[4] && !kp[4]) { dm = 2; ds = true; }
	for (Int i = 0; i < 7; ++i) kp[i] = kn[i];
	if (ww <= 0 || wh <= 0) return;
	glm::vec3 tgt(0, 0, 0);
	ceye = tgt + cdist * glm::vec3(cosf(cpitch) * sinf(cyaw), sinf(cpitch), cosf(cpitch) * cosf(cyaw));
	glm::mat4 view = glm::lookAt(ceye, tgt, glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(glm::radians(50.f), (Float32)ww / wh, 0.1f, 1000.f);
	proj[1][1] *= -1; vp = proj * view; time_sec += dt;
}
void RT::OnRender() { graph.Execute(); }
int main() { Window w; RT r(&w); w.InitWindow(); w.Run(&r); system("pause"); return 0; }
