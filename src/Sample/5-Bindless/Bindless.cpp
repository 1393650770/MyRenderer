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

Vector<UInt32> ReadShader(CONST String& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	CHECK_WITH_LOG(!file.is_open(), " App Error: fail to open the shader file! ")
	size_t fileSize = (size_t)file.tellg();
	Vector<UInt32> buffer(fileSize / sizeof(UInt32));
	file.seekg(0); file.read((char*)buffer.data(), fileSize); file.close();
	return std::move(buffer);
}

static float g_cam_angle = 0.0f;
static float g_cam_distance = 3.0f;

struct MainPassData : public RenderGraphPassDataBase
{
	// Skybox
	RenderPipelineState* sky_pipeline = nullptr;
	ShaderResourceBinding* sky_srb = nullptr;
	TextureAsset* sky_cubemap = nullptr;

	// PBR
	RenderPipelineState* pbr_pipeline = nullptr;
	ShaderResourceBinding* pbr_srb = nullptr;
	TextureAsset* basecolor_tex = nullptr, *normal_tex = nullptr, *aorm_tex = nullptr;
	TextureAsset* cubemap_tex = nullptr, *irradiance_tex = nullptr;
	Buffer* mvp_ubo = nullptr, *camera_ubo = nullptr, *material_ubo = nullptr;
	UInt32 bc_idx=0, n_idx=0, ao_idx=0, cm_idx=0, irr_idx=0, lut_idx=0;
	RHI::Vulkan::VK_BindlessManager* bindless_mgr = nullptr;
	Texture* fallback_lut = nullptr; // cleanup for the fallback IBL LUT

	void Release() {
		if (sky_srb) { delete sky_srb; sky_srb = nullptr; }
		if (pbr_srb) { delete pbr_srb; pbr_srb = nullptr; }
		if (sky_cubemap) { delete sky_cubemap; sky_cubemap = nullptr; }
		if (basecolor_tex) { delete basecolor_tex; basecolor_tex = nullptr; }
		if (normal_tex)    { delete normal_tex; normal_tex = nullptr; }
		if (aorm_tex)      { delete aorm_tex; aorm_tex = nullptr; }
		if (cubemap_tex)   { delete cubemap_tex; cubemap_tex = nullptr; }
		if (irradiance_tex){ delete irradiance_tex; irradiance_tex = nullptr; }
		if (mvp_ubo)      { delete mvp_ubo; mvp_ubo = nullptr; }
		if (camera_ubo)   { delete camera_ubo; camera_ubo = nullptr; }
		if (material_ubo) { delete material_ubo; material_ubo = nullptr; }
		if (fallback_lut) { delete fallback_lut; fallback_lut = nullptr; }
	}
	VIRTUAL ~MainPassData() { Release(); }
};

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

	graph.AddRenderPass<MainPassData>("MainPass", &graph, cmd_list,
		[&](MainPassData& d, RenderGraphPassBuilder&, CommandList*)
	{
		ShaderDesc sd; ShaderDataPayload sp;
		auto* vp = this->GetWindow()->GetViewport();
		Vector<Texture*> rtvs = { vp->GetCurrentBackBufferRTV() };
		Texture* dsv = vp->GetCurrentBackBufferDSV();

		// ====== SKYBOX pipeline ======
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		sp.data = ReadShader("Shader/skybox_test.vert.spv");
		Shader* sv = RHICreateShader(sd, sp);
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		sp.data = ReadShader("Shader/skybox_test.frag.spv");
		Shader* sps = RHICreateShader(sd, sp);
		RenderGraphiPipelineStateDesc pd{};
		pd.shaders[0]=sv; pd.shaders[1]=sps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		pd.render_targets = rtvs; pd.depth_stencil_view = dsv;
		pd.raster_state.sample_count = 1;
		pd.blend_state.render_targets.resize(1);
		d.sky_pipeline = RHICreateRenderPipelineState(pd);
		d.sky_pipeline->CreateShaderResourceBinding(d.sky_srb, true);
		d.sky_cubemap = new TextureAsset("Texture/Skybox/bolonga_lod.dds");
		delete sv; delete sps;

		// ====== PBR bindless pipeline ======
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		sp.data = ReadShader("Shader/pbr_bindless_demo.vert.spv");
		Shader* pv = RHICreateShader(sd, sp);
		sd.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		sp.data = ReadShader("Shader/pbr_mesh_bindless.frag.spv");
		Shader* pps = RHICreateShader(sd, sp);
		pd = {};
		pd.shaders[0]=pv; pd.shaders[1]=pps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		pd.render_targets = rtvs; pd.depth_stencil_view = dsv;
		pd.raster_state.sample_count = 1;
		pd.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pd.depth_stencil_state.depth_test_enable = true;
		pd.depth_stencil_state.depth_write_enable = true;
		pd.blend_state.render_targets.resize(1);
		d.pbr_pipeline = RHICreateRenderPipelineState(pd);
		d.pbr_pipeline->CreateShaderResourceBinding(d.pbr_srb, false);
		delete pv; delete pps;

		// PBR textures
		d.basecolor_tex  = new TextureAsset("Texture/pbr_stone/pbr_stone_base_color.dds");
		d.normal_tex     = new TextureAsset("Texture/pbr_stone/pbr_stone_normal.dds");
		d.aorm_tex       = new TextureAsset("Texture/pbr_stone/pbr_stone_aorm.dds");
		d.cubemap_tex    = new TextureAsset("Texture/Skybox/bolonga_lod.dds");
		d.irradiance_tex = new TextureAsset("Texture/Skybox/bolonga_irr.dds");
		auto Wait = [](TextureAsset* t, const char* n){while(!t->GetTexture()){}std::cout<<"  "<<n<<std::endl;};
		Wait(d.basecolor_tex,"basecolor"); Wait(d.normal_tex,"normal"); Wait(d.aorm_tex,"aorm");
		Wait(d.cubemap_tex,"cubemap"); Wait(d.irradiance_tex,"irradiance");

		// Bindless registration
		d.bindless_mgr = RHIGetBindlessManager();
		d.bc_idx  = d.bindless_mgr->AllocateTexture2DSlot(d.basecolor_tex->GetTexture());
		d.n_idx   = d.bindless_mgr->AllocateTexture2DSlot(d.normal_tex->GetTexture());
		d.ao_idx  = d.bindless_mgr->AllocateTexture2DSlot(d.aorm_tex->GetTexture());
		d.cm_idx  = d.bindless_mgr->AllocateTextureCubeSlot(d.cubemap_tex->GetTexture());
		d.irr_idx = d.bindless_mgr->AllocateTextureCubeSlot(d.irradiance_tex->GetTexture());
		{ // fallback IBL LUT
			TextureDesc td; td.format=ENUM_TEXTURE_FORMAT::BGRA8; td.width=2; td.height=2;
			td.type=ENUM_TEXTURE_TYPE::ENUM_TYPE_2D; td.usage=ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
			auto* fl=RHICreateTexture(td); TextureDataPayload lp;
				d.fallback_lut = fl;
			Char w[16]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
			lp.data=Vector<Char>(w,w+16); lp.width=2; lp.height=2; lp.format=ENUM_TEXTURE_FORMAT::BGRA8;
			lp.type=ENUM_TEXTURE_TYPE::ENUM_TYPE_2D; lp.layer_count=1; lp.mip_level=1; lp.depth=1;
			fl->UpdateTextureData(lp); d.lut_idx=d.bindless_mgr->AllocateTexture2DSlot(fl);
		}
		// UBOs
		BufferDesc ub; ub.type=ENUM_BUFFER_TYPE::Uniform; ub.size=256;
		d.mvp_ubo=RHICreateBuffer(ub); d.camera_ubo=RHICreateBuffer(ub); d.material_ubo=RHICreateBuffer(ub);
		BindlessMaterialDataGPU mat{}; mat.basecolorIndex=d.bc_idx; mat.normalIndex=d.n_idx;
		mat.aormIndex=d.ao_idx; mat.cubemapIndex=d.cm_idx; mat.irradianceIndex=d.irr_idx;
		mat.iblLutIndex=d.lut_idx; mat.metallicFactor=0; mat.roughnessFactor=0.5f;
		void* mp=RHIMapBuffer(d.material_ubo,ENUM_MAP_TYPE::Write,ENUM_MAP_FLAG::None);
		memcpy(mp,&mat,sizeof(mat)); RHIUnmapBuffer(d.material_ubo);
		std::cout << "  Bindless ready." << std::endl;
	},
		[=](CONST MainPassData& d, CommandList* cmd)
	{
		auto* vp = this->GetWindow()->GetViewport();
		Vector<Texture*> rtvs = { vp->GetCurrentBackBufferRTV() };
		Texture* dsv = vp->GetCurrentBackBufferDSV();

		// --- SKYBOX ---
		if (d.sky_cubemap->GetTexture()) {
			Vector<ClearValue> c; c.push_back(rtvs[0]->GetTextureDesc().clear_value);
			if(dsv) c.push_back(dsv->GetTextureDesc().clear_value);
			cmd->SetRenderTarget(rtvs, dsv, c, dsv != nullptr);
			cmd->SetGraphicsPipeline(d.sky_pipeline);
			d.sky_srb->SetResource("cubemap_sampler", d.sky_cubemap->GetTexture());
			cmd->SetShaderResourceBinding(d.sky_srb);
			cmd->Draw(DrawAttribute{6,1,0,0});
		}

		// --- PBR QUAD (LOAD, preserve skybox) ---
		g_cam_angle += 0.005f;
		float cx=sin(g_cam_angle)*g_cam_distance, cz=cos(g_cam_angle)*g_cam_distance;
		glm::vec3 eye(cx,0.8f,cz);
		glm::mat4 view=glm::lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,1,0));
		glm::mat4 proj=glm::perspective(glm::radians(60.f),1280.f/960.f,0.1f,100.f);
		{ glm::mat4 m=glm::rotate(glm::mat4(1),glm::radians(-20.f),glm::vec3(1,0,0));
			void* p=RHIMapBuffer(d.mvp_ubo,ENUM_MAP_TYPE::Write,ENUM_MAP_FLAG::None);
			glm::mat4* a=static_cast<glm::mat4*>(p); a[0]=m;a[1]=view;a[2]=proj;
			RHIUnmapBuffer(d.mvp_ubo); }
		{ struct C_{glm::vec3 p;float _0;glm::vec3 d;float _1;};
			void* p=RHIMapBuffer(d.camera_ubo,ENUM_MAP_TYPE::Write,ENUM_MAP_FLAG::None);
			C_* c=static_cast<C_*>(p); c->p=eye; c->d=glm::normalize(glm::vec3(0,0,0)-eye);
			c->_0=c->_1=0; RHIUnmapBuffer(d.camera_ubo); }
		{
			Vector<ClearValue> nc;
			cmd->SetRenderTarget(rtvs, dsv, nc, dsv != nullptr);
		}
		cmd->SetGraphicsPipeline(d.pbr_pipeline);
		d.pbr_srb->SetResource("mvp", d.mvp_ubo);
		d.pbr_srb->SetResource("cameraData", d.camera_ubo);
		d.pbr_srb->SetResource("g_Material", d.material_ubo);
		cmd->SetShaderResourceBinding(d.pbr_srb);
		cmd->Draw(DrawAttribute{6,1,0,0});
	});

	graph.Compile();
}

int main() {
	Window window; RenderTest render(&window);
	window.InitWindow(); window.Run(&render);
	system("pause"); return 0;
}
