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

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderTest, public MXRender::RenderInterface)
public:
	RenderTest(Window* in_window);
	RenderTest() MYDEFAULT;
	VIRTUAL ~RenderTest() MYDEFAULT;
	VIRTUAL void BeginRender() OVERRIDE FINAL;
	VIRTUAL void EndRender() OVERRIDE FINAL;
	VIRTUAL void BeginFrame() OVERRIDE FINAL;
	VIRTUAL void OnFrame() OVERRIDE FINAL;
	VIRTUAL void EndFrame() OVERRIDE FINAL;
	Window* GetWindow();
protected:
	RenderGraph graph;
	Window* window;
MYRENDERER_END_CLASS

struct TestData : public RenderGraphPassDataBase
{
	// ---- PBR bindless pipeline ----
	RenderPipelineState* pbr_pipeline = nullptr;
	ShaderResourceBinding* pbr_srb = nullptr;
	Buffer* mvp_ubo = nullptr;
	Buffer* camera_ubo = nullptr;
	Buffer* material_ubo = nullptr;

	// ---- Skybox pipeline ----
	RenderPipelineState* skybox_pipeline = nullptr;
	ShaderResourceBinding* skybox_srb = nullptr;
	TextureAsset* skybox_tex = nullptr;

	// ---- Shared textures ----
	TextureAsset* basecolor_tex = nullptr;
	TextureAsset* normal_tex = nullptr;
	TextureAsset* aorm_tex = nullptr;
	TextureAsset* cubemap_tex = nullptr;
	TextureAsset* irradiance_tex = nullptr;

	// Bindless slots
	UInt32 basecolor_idx = 0, normal_idx = 0, aorm_idx = 0;
	UInt32 cubemap_idx = 0, irradiance_idx = 0, ibl_lut_idx = 0;
	RHI::Vulkan::VK_BindlessManager* bindless_mgr = nullptr;

	void Release()
	{
		if (pbr_srb)     { delete pbr_srb; pbr_srb = nullptr; }
		if (skybox_srb)  { delete skybox_srb; skybox_srb = nullptr; }
		if (basecolor_tex)  { delete basecolor_tex; basecolor_tex = nullptr; }
		if (normal_tex)     { delete normal_tex; normal_tex = nullptr; }
		if (aorm_tex)       { delete aorm_tex; aorm_tex = nullptr; }
		if (cubemap_tex)    { delete cubemap_tex; cubemap_tex = nullptr; }
		if (irradiance_tex) { delete irradiance_tex; irradiance_tex = nullptr; }
		if (skybox_tex)     { delete skybox_tex; skybox_tex = nullptr; }
		if (mvp_ubo)        { delete mvp_ubo; mvp_ubo = nullptr; }
		if (camera_ubo)     { delete camera_ubo; camera_ubo = nullptr; }
		if (material_ubo)   { delete material_ubo; material_ubo = nullptr; }
	}
	VIRTUAL ~TestData() { Release(); }
};

static bool WaitForTexture(TextureAsset* tex, const char* name, int max_ms = 5000)
{
	if (!tex) return false;
	auto start = std::chrono::steady_clock::now();
	while (tex->GetTexture() == nullptr)
	{
		auto elapsed = std::chrono::steady_clock::now() - start;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > max_ms)
		{ std::cout << "  TIMEOUT waiting for: " << name << std::endl; return false; }
	}
	std::cout << "  Loaded: " << name << std::endl;
	return true;
}

void RenderTest::BeginRender()
{
	std::cout << "=== Bindless PBR Renderer ===" << std::endl;
	CommandList* cmd_list = RHIGetImmediateCommandList();

	graph.AddRenderPass<TestData>("MainPass", &graph, cmd_list,
		[&](TestData& d, RenderGraphPassBuilder& builder, CommandList*)
	{
		ShaderDesc shader_desc;
		ShaderDataPayload shader_data;
		RenderGraphiPipelineStateDesc pipe_desc;
		BufferDesc ubo_desc;
		static constexpr UInt32 UBO_SZ = 256;

		// ====== SKYBOX PIPELINE ======
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		shader_data.data = ReadShader("Shader/skybox_test.vert.spv");
		Shader* sky_vs = RHICreateShader(shader_desc, shader_data);
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		shader_data.data = ReadShader("Shader/skybox_test.frag.spv");
		Shader* sky_ps = RHICreateShader(shader_desc, shader_data);

		pipe_desc = {};
		pipe_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = sky_vs;
		pipe_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = sky_ps;
		pipe_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<Texture*> rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		Texture* dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		pipe_desc.render_targets = rtvs;
		pipe_desc.depth_stencil_view = dsv;
		pipe_desc.raster_state.sample_count = 1;
		pipe_desc.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pipe_desc.raster_state.depth_bias = 0.0f;
		pipe_desc.depth_stencil_state.depth_test_enable = true;
		pipe_desc.depth_stencil_state.depth_write_enable = false; // skybox doesn't write depth
		pipe_desc.depth_stencil_state.depth_func = ENUM_STENCIL_FUNCTION::ENUM_LEQUAL;
		pipe_desc.blend_state.render_targets.resize(rtvs.size());
		d.skybox_pipeline = RHICreateRenderPipelineState(pipe_desc);
		d.skybox_pipeline->CreateShaderResourceBinding(d.skybox_srb, false);


		// Skybox cubemap
		d.skybox_tex = new TextureAsset("Texture/Skybox/bolonga_lod.dds");

		delete sky_vs; delete sky_ps;

		// ====== PBR BINDLESS PIPELINE ======
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		shader_data.data = ReadShader("Shader/pbr_bindless_demo.vert.spv");
		Shader* pbr_vs = RHICreateShader(shader_desc, shader_data);
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		shader_data.data = ReadShader("Shader/pbr_mesh_bindless.frag.spv");
		Shader* pbr_ps = RHICreateShader(shader_desc, shader_data);

		pipe_desc = {};
		pipe_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = pbr_vs;
		pipe_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = pbr_ps;
		pipe_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		pipe_desc.render_targets = rtvs;
		pipe_desc.depth_stencil_view = dsv;
		pipe_desc.raster_state.sample_count = 1;
		pipe_desc.raster_state.cull_mode = ENUM_RASTER_CULLMODE::None;
		pipe_desc.raster_state.depth_bias = -1.0f;
		pipe_desc.depth_stencil_state.depth_test_enable = true;
		pipe_desc.depth_stencil_state.depth_write_enable = true;
		pipe_desc.blend_state.render_targets.resize(rtvs.size());
		d.pbr_pipeline = RHICreateRenderPipelineState(pipe_desc);
		d.pbr_pipeline->CreateShaderResourceBinding(d.pbr_srb, false);

		// PBR textures
		d.basecolor_tex  = new TextureAsset("Texture/pbr_stone/pbr_stone_base_color.dds");
		d.normal_tex     = new TextureAsset("Texture/pbr_stone/pbr_stone_normal.dds");
		d.aorm_tex       = new TextureAsset("Texture/pbr_stone/pbr_stone_aorm.dds");
		d.cubemap_tex    = new TextureAsset("Texture/Skybox/bolonga_lod.dds");
		d.irradiance_tex = new TextureAsset("Texture/Skybox/bolonga_irr.dds");

		// Wait for all textures
		WaitForTexture(d.basecolor_tex, "pbr_stone_base_color");
		WaitForTexture(d.normal_tex, "pbr_stone_normal");
		WaitForTexture(d.aorm_tex, "pbr_stone_aorm");
		WaitForTexture(d.cubemap_tex, "bolonga_lod");
		WaitForTexture(d.irradiance_tex, "bolonga_irr");
		WaitForTexture(d.skybox_tex, "skybox_lod");

		// Register with bindless
		d.bindless_mgr = RHIGetBindlessManager();
		if (d.bindless_mgr && d.bindless_mgr->IsEnabled())
		{
			d.basecolor_idx  = d.bindless_mgr->AllocateTexture2DSlot(d.basecolor_tex->GetTexture());
			d.normal_idx     = d.bindless_mgr->AllocateTexture2DSlot(d.normal_tex->GetTexture());
			d.aorm_idx       = d.bindless_mgr->AllocateTexture2DSlot(d.aorm_tex->GetTexture());
			d.cubemap_idx    = d.bindless_mgr->AllocateTextureCubeSlot(d.cubemap_tex->GetTexture());
			d.irradiance_idx = d.bindless_mgr->AllocateTextureCubeSlot(d.irradiance_tex->GetTexture());

			// Fallback IBL LUT (2x2 white)
			{
				TextureDesc lut_desc;
				lut_desc.format = ENUM_TEXTURE_FORMAT::BGRA8;
				lut_desc.width = 2; lut_desc.height = 2;
				lut_desc.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
				lut_desc.usage = ENUM_TEXTURE_USAGE_TYPE::ENUM_TYPE_SHADERRESOURCE;
				auto* fallback_lut = RHICreateTexture(lut_desc);
				TextureDataPayload lut_data;
				Char white[16] = { -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1 };
				lut_data.data = Vector<Char>(white, white + 16);
				lut_data.width = 2; lut_data.height = 2;
				lut_data.format = ENUM_TEXTURE_FORMAT::BGRA8;
				lut_data.type = ENUM_TEXTURE_TYPE::ENUM_TYPE_2D;
				lut_data.layer_count = 1; lut_data.mip_level = 1; lut_data.depth = 1;
				fallback_lut->UpdateTextureData(lut_data);
				d.ibl_lut_idx = d.bindless_mgr->AllocateTexture2DSlot(fallback_lut);
			}
			std::cout << "Bindless slots:" << std::endl;
			std::cout << "  basecolor:" << d.basecolor_idx << " normal:" << d.normal_idx
				<< " aorm:" << d.aorm_idx << std::endl;
			std::cout << "  cubemap:" << d.cubemap_idx << " irradiance:" << d.irradiance_idx
				<< " ibl_lut:" << d.ibl_lut_idx << std::endl;
		}
		else { std::cout << "WARNING: Bindless unavailable!" << std::endl; }

		// MaterialData UBO (Set 1 binding 0)
		BindlessMaterialDataGPU mat{};
		mat.basecolorIndex = d.basecolor_idx; mat.normalIndex = d.normal_idx;
		mat.aormIndex = d.aorm_idx; mat.cubemapIndex = d.cubemap_idx;
		mat.irradianceIndex = d.irradiance_idx; mat.iblLutIndex = d.ibl_lut_idx;
		mat.metallicFactor = 0.0f; mat.roughnessFactor = 0.5f;

		ubo_desc.size = UBO_SZ; ubo_desc.type = ENUM_BUFFER_TYPE::Uniform;
		d.material_ubo = RHICreateBuffer(ubo_desc);
		void* m = RHIMapBuffer(d.material_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
		memcpy(m, &mat, sizeof(mat)); RHIUnmapBuffer(d.material_ubo);

		// MVP + Camera UBOs
		ubo_desc.size = UBO_SZ;
		d.mvp_ubo = RHICreateBuffer(ubo_desc);
		d.camera_ubo = RHICreateBuffer(ubo_desc);

		delete pbr_vs; delete pbr_ps;
		std::cout << "Setup complete." << std::endl;
	},
		[=](CONST TestData& d, CommandList* cmd)
	{
		if (!d.bindless_mgr || !d.bindless_mgr->IsEnabled()) return;

		// Update camera
		g_cam_angle += 0.005f;
		float cx = sin(g_cam_angle) * g_cam_distance;
		float cz = cos(g_cam_angle) * g_cam_distance;
		glm::vec3 eye(cx, 0.8f, cz);
		glm::vec3 center(0.0f, 0.0f, 0.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::mat4 view = glm::lookAt(eye, center, up);
		float aspect = 1280.0f / 960.0f;
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

		// Upload PBR MVP
		{
			glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-20.0f), glm::vec3(1,0,0));
			void* m = RHIMapBuffer(d.mvp_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
			glm::mat4* mats = static_cast<glm::mat4*>(m);
			mats[0] = model; mats[1] = view; mats[2] = proj;
			RHIUnmapBuffer(d.mvp_ubo);
		}
		// Upload CameraData
		{
			struct Cam { glm::vec3 p; float pad0; glm::vec3 d; float pad1; };
			void* m = RHIMapBuffer(d.camera_ubo, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::None);
			Cam* c = static_cast<Cam*>(m);
			c->p = eye; c->d = glm::normalize(center - eye); c->pad0 = c->pad1 = 0;
			RHIUnmapBuffer(d.camera_ubo);
		}

		// RTV / DSV
		Vector<Texture*> rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		Texture* dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();

		// Manual clear via vkCmdClearColorImage (bypasses broken render pass clear)
		cmd->ClearTexture(rtvs[0], { 0.1f, 0.15f, 0.3f, 1.0f });
		Vector<ClearValue> no_clear; // empty → LOAD_OP_LOAD

		// ====== PASS 1: SKYBOX ======
		{
			cmd->SetRenderTarget(rtvs, dsv, no_clear, dsv != nullptr);
			cmd->SetGraphicsPipeline(d.skybox_pipeline);
			d.skybox_srb->SetResource("cubemap_sampler", d.skybox_tex->GetTexture());
			cmd->SetShaderResourceBinding(d.skybox_srb);
			cmd->Draw(DrawAttribute{36, 1, 0, 0});
		}

		// ====== PASS 2: PBR QUAD ======
		{
			cmd->SetRenderTarget(rtvs, dsv, no_clear, dsv != nullptr);
			cmd->SetGraphicsPipeline(d.pbr_pipeline);
			d.pbr_srb->SetResource("mvp", d.mvp_ubo);
			d.pbr_srb->SetResource("cameraData", d.camera_ubo);
			d.pbr_srb->SetResource("g_Material", d.material_ubo);
			cmd->SetShaderResourceBinding(d.pbr_srb);
			cmd->Draw(DrawAttribute{6, 1, 0, 0});
		}
	});

	graph.Compile();
}

void RenderTest::EndRender() { graph.Release(); }
void RenderTest::BeginFrame() {}
void RenderTest::OnFrame()  { graph.Execute(); }
void RenderTest::EndFrame() {}

RenderTest::RenderTest(Window* in_window) : window(in_window) {}
MXRender::Application::Window* RenderTest::GetWindow() { return window; }

int main()
{
	Window window;
	RenderTest render(&window);
	window.InitWindow();
	window.Run(&render);
	system("pause");
	return 0;
}
