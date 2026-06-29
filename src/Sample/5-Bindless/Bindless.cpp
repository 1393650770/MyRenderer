#include <iostream>
#include <fstream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "Asset/TextureAsset.h"
#include "Render/Core/BindlessMaterialData.h"
#include "RHI/Vulkan/VK_BindlessManager.h"
using namespace MXRender::Asset;
using namespace MXRender::RHI;
using namespace MXRender::Render;
using namespace MXRender::Application;
using namespace MXRender;

Vector<UInt32> ReadShader(CONST String& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!file.is_open(), " App Error: fail to open the shader file! ")
	size_t fileSize = (size_t)file.tellg();
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();
	return std::move(buffer);
}

static float g_cam_angle = 0.0f;
static float g_cam_distance = 3.0f;

// ====== Skybox pass data ======
struct SkyboxData : public RenderGraphPassDataBase
{
	RenderPipelineState* pipeline = nullptr;
	ShaderResourceBinding* srb = nullptr;
	TextureAsset* cubemap_tex = nullptr;
	VIRTUAL ~SkyboxData() { Release(); }
	void Release() {
		if (srb) { delete srb; srb = nullptr; }
		if (cubemap_tex) { delete cubemap_tex; cubemap_tex = nullptr; }
	}
};

// ====== PBR pass data ======
struct PBRData : public RenderGraphPassDataBase
{
	RenderPipelineState* pipeline = nullptr;
	ShaderResourceBinding* srb = nullptr;
	TextureAsset* basecolor_tex = nullptr;
	TextureAsset* normal_tex = nullptr;
	TextureAsset* aorm_tex = nullptr;
	TextureAsset* cubemap_tex = nullptr;
	TextureAsset* irradiance_tex = nullptr;
	Buffer* mvp_ubo = nullptr;
	Buffer* camera_ubo = nullptr;
	Buffer* material_ubo = nullptr;
	UInt32 basecolor_idx = 0, normal_idx = 0, aorm_idx = 0;
	UInt32 cubemap_idx = 0, irradiance_idx = 0, ibl_lut_idx = 0;
	RHI::Vulkan::VK_BindlessManager* bindless_mgr = nullptr;
	VIRTUAL ~PBRData() { Release(); }
	void Release() {
		if (srb) { delete srb; srb = nullptr; }
		if (basecolor_tex)  { delete basecolor_tex; basecolor_tex = nullptr; }
		if (normal_tex)     { delete normal_tex; normal_tex = nullptr; }
		if (aorm_tex)       { delete aorm_tex; aorm_tex = nullptr; }
		if (cubemap_tex)    { delete cubemap_tex; cubemap_tex = nullptr; }
		if (irradiance_tex) { delete irradiance_tex; irradiance_tex = nullptr; }
		if (mvp_ubo)       { delete mvp_ubo; mvp_ubo = nullptr; }
		if (camera_ubo)    { delete camera_ubo; camera_ubo = nullptr; }
		if (material_ubo)  { delete material_ubo; material_ubo = nullptr; }
	}
};

static bool WaitFor(TextureAsset* t, const char* name, int ms = 5000) {
	if (!t) return false;
	auto start = std::chrono::steady_clock::now();
	while (t->GetTexture() == nullptr) {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start).count() > ms)
		{ std::cout << "  TIMEOUT: " << name << std::endl; return false; }
	}
	std::cout << "  Loaded: " << name << std::endl;
	return true;
}

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderTest, public MXRender::RenderInterface)
public:
	RenderTest(Window* w) : window(w) {}
	RenderTest() MYDEFAULT;
	~RenderTest() MYDEFAULT;
	void BeginRender() OVERRIDE FINAL;
	void EndRender() OVERRIDE FINAL { graph.Release(); }
	void BeginFrame() OVERRIDE FINAL {}
	void OnFrame() OVERRIDE FINAL { graph.Execute(); }
	void EndFrame() OVERRIDE FINAL {}
	Window* GetWindow() { return window; }
protected:
	RenderGraph graph;
	Window* window;
MYRENDERER_END_CLASS

void RenderTest::BeginRender()
{
	std::cout << "=== Bindless PBR ===" << std::endl;
	CommandList* cmd_list = RHIGetImmediateCommandList();

	// ====== PASS 1: SKYBOX ======
	graph.AddRenderPass<SkyboxData>("SkyboxPass", &graph, cmd_list,
		[&](SkyboxData& d, RenderGraphPassBuilder&, CommandList*)
	{
		Shader* vs, *ps;
		ShaderDesc sd; ShaderDataPayload sp;
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		sp.data = ReadShader("Shader/skybox_test.vert.spv");
		vs = RHICreateShader(sd, sp);
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		sp.data = ReadShader("Shader/skybox_test.frag.spv");
		ps = RHICreateShader(sd, sp);
		RenderGraphiPipelineStateDesc pd{};
		pd.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = vs;
		pd.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		auto* vp = this->GetWindow()->GetViewport();
		pd.render_targets = { vp->GetCurrentBackBufferRTV() };
		pd.depth_stencil_view = vp->GetCurrentBackBufferDSV();
		pd.raster_state.sample_count = 1;
		pd.blend_state.render_targets.resize(1);
		d.pipeline = RHICreateRenderPipelineState(pd);
		d.pipeline->CreateShaderResourceBinding(d.srb, true);
		d.cubemap_tex = new TextureAsset("Texture/Skybox/bolonga_lod.dds");
		delete vs; delete ps;
	},
		[=](CONST SkyboxData& d, CommandList* cmd)
	{
		if (d.cubemap_tex->GetTexture())
		{
			auto* vp = this->GetWindow()->GetViewport();
			Vector<Texture*> rtvs = { vp->GetCurrentBackBufferRTV() };
			Texture* dsv = vp->GetCurrentBackBufferDSV();
			Vector<ClearValue> clear_values;
			clear_values.push_back(rtvs[0]->GetTextureDesc().clear_value);
			if (dsv) clear_values.push_back(dsv->GetTextureDesc().clear_value);
			cmd->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
			cmd->SetGraphicsPipeline(d.pipeline);
			d.srb->SetResource("cubemap_sampler", d.cubemap_tex->GetTexture());
			cmd->SetShaderResourceBinding(d.srb);
			cmd->Draw(DrawAttribute{6, 1, 0, 0});
		}
	});

	// ====== PASS 2: PBR BINDLESS ======
	graph.AddRenderPass<PBRData>("PBRPass", &graph, cmd_list,
		[&](PBRData& d, RenderGraphPassBuilder&, CommandList*)
	{
		ShaderDesc sd; ShaderDataPayload sp;
		// Vertex shader: procedural quad
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		sp.data = ReadShader("Shader/pbr_bindless_demo.vert.spv");
		Shader* vs = RHICreateShader(sd, sp);
		// Fragment shader: bindless PBR
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		sp.data = ReadShader("Shader/pbr_mesh_bindless.frag.spv");
		Shader* ps = RHICreateShader(sd, sp);
		RenderGraphiPipelineStateDesc pd{};
		pd.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = vs;
		pd.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		auto* vp = this->GetWindow()->GetViewport();
		pd.render_targets = { vp->GetCurrentBackBufferRTV() };
		pd.depth_stencil_view = vp->GetCurrentBackBufferDSV();
		pd.raster_state.sample_count = 1;
		pd.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pd.depth_stencil_state.depth_test_enable = true;
		pd.depth_stencil_state.depth_write_enable = true;
		pd.blend_state.render_targets.resize(1);
		d.pipeline = RHICreateRenderPipelineState(pd);
		d.pipeline->CreateShaderResourceBinding(d.srb, false);
		std::cout << "  PBR pipeline=" << (void*)d.pipeline << " srb=" << (void*)d.srb << std::endl;

		// Load textures
		d.basecolor_tex  = new TextureAsset("Texture/pbr_stone/pbr_stone_base_color.dds");
		d.normal_tex     = new TextureAsset("Texture/pbr_stone/pbr_stone_normal.dds");
		d.aorm_tex       = new TextureAsset("Texture/pbr_stone/pbr_stone_aorm.dds");
		d.cubemap_tex    = new TextureAsset("Texture/Skybox/bolonga_lod.dds");
		d.irradiance_tex = new TextureAsset("Texture/Skybox/bolonga_irr.dds");
		WaitFor(d.basecolor_tex, "pbr_stone_base_color");
		WaitFor(d.normal_tex, "pbr_stone_normal");
		WaitFor(d.aorm_tex, "pbr_stone_aorm");
		WaitFor(d.cubemap_tex, "bolonga_lod_pbr");
		WaitFor(d.irradiance_tex, "bolonga_irr");

		// Register with bindless manager
		d.bindless_mgr = RHIGetBindlessManager();
		if (d.bindless_mgr && d.bindless_mgr->IsEnabled()) {
			d.basecolor_idx  = d.bindless_mgr->AllocateTexture2DSlot(d.basecolor_tex->GetTexture());
			d.normal_idx     = d.bindless_mgr->AllocateTexture2DSlot(d.normal_tex->GetTexture());
			d.aorm_idx       = d.bindless_mgr->AllocateTexture2DSlot(d.aorm_tex->GetTexture());
			d.cubemap_idx    = d.bindless_mgr->AllocateTextureCubeSlot(d.cubemap_tex->GetTexture());
			d.irradiance_idx = d.bindless_mgr->AllocateTextureCubeSlot(d.irradiance_tex->GetTexture());
			// Fallback IBL LUT
			TextureDesc lutd; lutd.format = ENUM_TEXTURE_FORMAT::BGRA8;
			lutd.width = 2; lutd.height = 2; lutd.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
			lutd.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
			auto* fl = RHICreateTexture(lutd);
			TextureDataPayload lp; Char w[16]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
			lp.data = Vector<Char>(w,w+16); lp.width=2; lp.height=2;
			lp.format = ENUM_TEXTURE_FORMAT::BGRA8; lp.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
			lp.layer_count=1; lp.mip_level=1; lp.depth=1;
			fl->UpdateTextureData(lp);
			d.ibl_lut_idx = d.bindless_mgr->AllocateTexture2DSlot(fl);
		} else { std::cout << "  WARNING: Bindless unavailable!" << std::endl; }

		// UBOs
		BufferDesc ub; ub.type = ENUM_BUFFER_TYPE::Uniform; ub.size = 256;
		d.mvp_ubo = RHICreateBuffer(ub);
		d.camera_ubo = RHICreateBuffer(ub);
		d.material_ubo = RHICreateBuffer(ub);
		BindlessMaterialDataGPU mat{};
		mat.basecolorIndex = d.basecolor_idx; mat.normalIndex = d.normal_idx;
		mat.aormIndex = d.aorm_idx; mat.cubemapIndex = d.cubemap_idx;
		mat.irradianceIndex = d.irradiance_idx; mat.iblLutIndex = d.ibl_lut_idx;
		mat.metallicFactor = 0.0f; mat.roughnessFactor = 0.5f;
		void* mp = RHIMapBuffer(d.material_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
		memcpy(mp, &mat, sizeof(mat)); RHIUnmapBuffer(d.material_ubo);

		d.srb->SetResource("mvp", d.mvp_ubo); // TEST in setup
		delete vs; delete ps;
		std::cout << "  PBR pass ready." << std::endl;
	},
		[=](CONST PBRData& d, CommandList* cmd)
	{
		if (!d.bindless_mgr || !d.bindless_mgr->IsEnabled()) return;
		g_cam_angle += 0.005f;
		float cx = sin(g_cam_angle) * g_cam_distance;
		float cz = cos(g_cam_angle) * g_cam_distance;
		glm::vec3 eye(cx, 0.8f, cz);
		glm::mat4 view = glm::lookAt(eye, glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1280.0f/960.0f, 0.1f, 100.0f);
		// MVP
		{ glm::mat4 m = glm::rotate(glm::mat4(1), glm::radians(-20.0f), glm::vec3(1,0,0));
			void* p = RHIMapBuffer(d.mvp_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
			glm::mat4* a = static_cast<glm::mat4*>(p); a[0]=m; a[1]=view; a[2]=proj;
			RHIUnmapBuffer(d.mvp_ubo); }
		// Camera
		{ struct C{glm::vec3 p; float pad0; glm::vec3 d; float pad1;};
			void* p = RHIMapBuffer(d.camera_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
			C* c = static_cast<C*>(p); c->p=eye; c->d=glm::normalize(glm::vec3(0,0,0)-eye);
			c->pad0=c->pad1=0; RHIUnmapBuffer(d.camera_ubo); }
		// Draw
		auto* vp = this->GetWindow()->GetViewport();
		Vector<Texture*> rtvs = { vp->GetCurrentBackBufferRTV() };
		Texture* dsv = vp->GetCurrentBackBufferDSV();
		Vector<ClearValue> nc;
		cmd->SetRenderTarget(rtvs, dsv, nc, dsv != nullptr);
		cmd->SetGraphicsPipeline(d.pipeline);
		d.srb->SetResource("mvp", d.mvp_ubo);
		d.srb->SetResource("cameraData", d.camera_ubo);
		d.srb->SetResource("g_Material", d.material_ubo);
		cmd->SetShaderResourceBinding(d.srb);
		cmd->Draw(DrawAttribute{6, 1, 0, 0});
	});

	graph.Compile();
}

int main()
{
	Window window;
	RenderTest render(&window);
	window.InitWindow();
	window.Run(&render);
	system("pause");
	return 0;
}
