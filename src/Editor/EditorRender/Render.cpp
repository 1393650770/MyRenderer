#include "Render.h"

#include "Render/Core/RenderGraphPass.h"
#include "RHI/RenderPipelineState.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderShader.h"
#include "Asset/TextureAsset.h"
#include <iostream>
#include <fstream>
#include "Application/Window.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderUtils.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"
#include "Render/Core/RenderGraphBuilder.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)


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

void EditorRenderPipeline::OnInit(Application::Window* in_window)
{
	window = in_window;
	std::cout << "Hello Editor" << std::endl;
	editor_ui.Init(window);
	RHI::CommandList* cmd_list = RHIGetImmediateCommandList();

	// Register backbuffer as retained (imported) resources so the RDG knows about them.
	// This enables the editor to visualize pass-resource dependencies.
	RHI::Texture* backbuffer_rtv = window->GetViewport()->GetCurrentBackBufferRTV();
	RHI::Texture* backbuffer_dsv = window->GetViewport()->GetCurrentBackBufferDSV();

	RHI::TextureDesc rt_desc = backbuffer_rtv->GetTextureDesc();
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* rt_resource =
		graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>("BackBuffer", rt_desc, backbuffer_rtv);

	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* ds_resource = nullptr;
	if (backbuffer_dsv)
	{
		RHI::TextureDesc ds_desc = backbuffer_dsv->GetTextureDesc();
		ds_resource = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>("DepthStencil", ds_desc, backbuffer_dsv);
	}

	struct ClearPassData :public  Render::RenderGraphPassDataBase
	{
		VIRTUAL ~ClearPassData()
		{
			Release();
		}
		void Release()
		{

		}
	};
	graph.AddRenderPass<ClearPassData>("ClearPass", &graph, cmd_list,
		[&](ClearPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
	{
		// Declare: ClearPass writes to BackBuffer and DepthStencil
		builder.Write(rt_resource);
		if (ds_resource) builder.Write(ds_resource);
	},
		[=](CONST ClearPassData& data, RHI::CommandList* in_cmd_list)
	{

		Vector<RHI::ClearValue> clear_values;
		Vector<RHI::Texture*> rtvs;
		RHI::Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		for (auto rtv : rtvs)
		{
			clear_values.push_back(rtv->GetTextureDesc().clear_value);
		}
		if (dsv)
			clear_values.push_back(dsv->GetTextureDesc().clear_value);
		in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
		for (auto rtv : rtvs)
		{
			in_cmd_list->ClearTexture(rtv);
		}
		if (dsv)
			in_cmd_list->ClearTexture(dsv);

	});


	struct TestData :public  Render::RenderGraphPassDataBase
	{

		RHI::RenderPipelineState* pipeline_state = nullptr;
		RHI::ShaderResourceBinding* srb = nullptr;
		Asset::TextureAsset* bind_texture = nullptr;
		RHI::Buffer* const_buffer = nullptr;
		VIRTUAL ~TestData()
		{
			Release();
		}
		void Release()
		{
			if (srb)
			{
				delete srb;
				srb = nullptr;
			}
			if (bind_texture)
			{
				delete bind_texture;
				bind_texture = nullptr;
			}
			if (const_buffer)
			{
				delete const_buffer;
				const_buffer = nullptr;
			}
		}
	};

	graph.AddRenderPass<TestData>("TestPass", &graph, cmd_list,
		[&](TestData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList* in_cmd_list)
	{
		// TestPass: reads depth for testing, writes to backbuffer and depth
		builder.Write(rt_resource);
		if (ds_resource) builder.Read(ds_resource);
		if (ds_resource) builder.Write(ds_resource);

		RHI::Shader* vs_shader;
		RHI::Shader* ps_shader;
		RHI::ShaderDesc shader_desc;
		RHI::ShaderDataPayload shader_data;
		RHI::RenderGraphiPipelineStateDesc pipeline_state_desc;
		RHI::TextureDesc texture_desc;
		RHI::TextureDataPayload texture_data;
		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex;
		shader_desc.shader_name = "TestVS";
		shader_data.data = ReadShader("Shader/texture_test.vert.spv");
		vs_shader = RHICreateShader(shader_desc, shader_data);

		shader_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel;
		shader_desc.shader_name = "TestPS";
		shader_data.data = ReadShader("Shader/stipple_transparency_test.frag.spv");
		ps_shader = RHICreateShader(shader_desc, shader_data);

		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = vs_shader;
		pipeline_state_desc.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = ps_shader;
		pipeline_state_desc.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		Vector<RHI::Texture*> rtvs;
		RHI::Texture* dsv;
		rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
		pipeline_state_desc.render_targets = rtvs;
		pipeline_state_desc.depth_stencil_view = dsv;
		pipeline_state_desc.raster_state.sample_count = 1;
		pipeline_state_desc.blend_state.render_targets.resize(rtvs.size());

		data.pipeline_state = RHICreateRenderPipelineState(pipeline_state_desc);
		data.pipeline_state->CreateShaderResourceBinding(data.srb, true);

		data.bind_texture = new Asset::TextureAsset("Texture/pbr_stone/pbr_stone_aorm.dds");
		delete vs_shader;
		delete ps_shader;
		RHI::BufferDesc bufferdesc;
		bufferdesc.size = sizeof(float);
		bufferdesc.stride = sizeof(float);
		bufferdesc.type = ENUM_BUFFER_TYPE::Uniform;
		data.const_buffer = RHICreateBuffer(bufferdesc);
		data.srb->SetResource("constants", data.const_buffer);
	},
		[=](CONST TestData& data, RHI::CommandList* in_cmd_list)
	{
		if (data.bind_texture->GetTexture())
		{
			Vector<RHI::ClearValue> clear_values;
			Vector<RHI::Texture*> rtvs;
			RHI::Texture* dsv;
			rtvs = { this->GetWindow()->GetViewport()->GetCurrentBackBufferRTV() };
			dsv = this->GetWindow()->GetViewport()->GetCurrentBackBufferDSV();
			for (auto rtv : rtvs)
			{
				clear_values.push_back(rtv->GetTextureDesc().clear_value);
			}
			if (dsv)
				clear_values.push_back(dsv->GetTextureDesc().clear_value);
			in_cmd_list->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
			in_cmd_list->SetGraphicsPipeline(data.pipeline_state);
			data.srb->SetResource("basecolor_sampler", data.bind_texture->GetTexture());

			struct Constant
			{
				float z = 0.0f;
			};
			{
				RHI::MapHelper<Constant> map_helper(data.const_buffer, ENUM_MAP_TYPE::Write, ENUM_MAP_FLAG::Discard);
				map_helper->z = 1.0f;
			}

			in_cmd_list->SetShaderResourceBinding(data.srb);
			DrawAttribute draw_attr;
			draw_attr.vertexCount = 6;
			draw_attr.instanceCount = 1;
			in_cmd_list->Draw(draw_attr);
		}
	});

	editor_ui.SetRenderGraph(&graph);
	editor_ui.AddPass(&graph);
	graph.Compile();
	// Sync runtime graph to editor visualization
	if (auto* rgp = editor_ui.GetRenderGraphPanel())
		rgp->SyncRuntimeToEditor(&graph);

	// --  Register fallback shaders for loaded graph passes
	InitRenderPasses();
}

// -- 
void EditorRenderPipeline::InitRenderPasses()
{
	// ---- Skybox pipeline ----
	{
		RHI::ShaderDesc vs_desc; vs_desc.shader_type = ENUM_SHADER_STAGE::Shader_Vertex; vs_desc.shader_name = "SkyboxVS";
		RHI::ShaderDataPayload vs_data; vs_data.data = ReadShader("Shader/skybox_test.vert.spv");
		skybox_vs = RHICreateShader(vs_desc, vs_data);
		RHI::ShaderDesc ps_desc; ps_desc.shader_type = ENUM_SHADER_STAGE::Shader_Pixel; ps_desc.shader_name = "SkyboxPS";
		RHI::ShaderDataPayload ps_data; ps_data.data = ReadShader("Shader/skybox_test.frag.spv");
		skybox_ps = RHICreateShader(ps_desc, ps_data);
		RHI::RenderGraphiPipelineStateDesc pd; pd.shaders[ENUM_SHADER_STAGE::Shader_Vertex] = skybox_vs; pd.shaders[ENUM_SHADER_STAGE::Shader_Pixel] = skybox_ps;
		pd.primitive_topology = ENUM_PRIMITIVE_TYPE::TriangleList;
		pd.render_targets = { window->GetViewport()->GetCurrentBackBufferRTV() };
		pd.depth_stencil_view = window->GetViewport()->GetCurrentBackBufferDSV();
		pd.raster_state.sample_count = 1; pd.blend_state.render_targets.resize(1);
		skybox_pipeline = RHICreateRenderPipelineState(pd);
		skybox_pipeline->CreateShaderResourceBinding(skybox_srb, true);
	}
	Render::RenderGraphBuilder::RegisterPassExecute("SkyboxPass",
		[this](RHI::CommandList* cmd, Map<String, Render::RenderGraphResourceBase*>& res) {
			//   Get current swapchain image each frame (swapchain rotates images)
			auto* rt_tex = this->window->GetViewport()->GetCurrentBackBufferRTV();
			auto* ds_tex = this->window->GetViewport()->GetCurrentBackBufferDSV();
			if (!rt_tex) return;
			Vector<RHI::ClearValue> cvs = { RHI::ClearValue{0.2f,0.2f,0.3f,1.0f} };
			if (ds_tex) cvs.push_back(RHI::ClearValue{1.0f,0});
			cmd->SetRenderTarget({rt_tex}, ds_tex, cvs, ds_tex != nullptr);
			if (auto* cm = res["SkyboxCubemap"]) { if (auto* t = cm->GetAsTexture()) skybox_srb->SetResource("cubemap_sampler", t); }
			cmd->SetGraphicsPipeline(skybox_pipeline); cmd->SetShaderResourceBinding(skybox_srb);
			cmd->Draw(DrawAttribute{6,1,0,0});
		});
	Render::RenderGraphBuilder::RegisterPassExecute("PBRPass",
		[this](RHI::CommandList* cmd, Map<String, Render::RenderGraphResourceBase*>& res) {
			//   Get current swapchain image each frame (swapchain rotates images)
			auto* rt_tex = this->window->GetViewport()->GetCurrentBackBufferRTV();
			auto* ds_tex = this->window->GetViewport()->GetCurrentBackBufferDSV();
			if (!rt_tex) return;
			cmd->SetRenderTarget({rt_tex}, ds_tex, {}, ds_tex != nullptr);
			cmd->SetGraphicsPipeline(skybox_pipeline); cmd->SetShaderResourceBinding(skybox_srb);
			cmd->Draw(DrawAttribute{6,1,0,0});
		});
}



void EditorRenderPipeline::RebuildFromDefinition(CONST MXRender::Render::RenderGraphDefinition& def)
{
	if (!window) return;
	Vector<Render::ExternalResourceBinding> externals;
	RHI::Texture* backbuffer_rtv = window->GetViewport()->GetCurrentBackBufferRTV();
	RHI::Texture* backbuffer_dsv = window->GetViewport()->GetCurrentBackBufferDSV();
	externals.push_back({"BackBuffer", backbuffer_rtv, nullptr});
	if (backbuffer_dsv) externals.push_back({"DepthStencil", backbuffer_dsv, nullptr});
	if (Render::RenderGraphBuilder::BuildRuntimeGraph(def, &graph, externals))
	{
		editor_ui.AddPass(&graph);
		graph.Compile();
		if (auto* rgp = editor_ui.GetRenderGraphPanel()) rgp->SyncRuntimeToEditor(&graph);
	}
}

void EditorRenderPipeline::PostFrame()
{
	if (!has_deferred_rebuild) return;
	has_deferred_rebuild = false;
	RebuildFromDefinition(deferred_def);
	std::cout << "[EditorRenderPipeline] PostFrame rebuild: " << deferred_def.graph_name << std::endl;
}

void EditorRenderPipeline::OnShutdown()
{
	if (skybox_srb) { delete skybox_srb; skybox_srb = nullptr; }
	if (skybox_pipeline) { delete skybox_pipeline; skybox_pipeline = nullptr; }
	if (skybox_vs) { delete skybox_vs; skybox_vs = nullptr; }
	if (skybox_ps) { delete skybox_ps; skybox_ps = nullptr; }

	editor_ui.Release();
	graph.Release();
}

void EditorRenderPipeline::OnUpdate(float dt)
{
	//   Process editor command queue — execute all commands collected during the
	// previous frame's ImGui Draw(). This separates logic (OnUpdate) from rendering (OnRender).
	auto* rgp = editor_ui.GetRenderGraphPanel();
	if (rgp)
	{
		rgp->GetCommandQueue().ProcessAll(rgp->GetCommandHistory());
	}

	//   Tick debounced graph-modified event
	if (rgp->HasPendingBuild()) { deferred_def = rgp->GetPendingBuildDef(); has_deferred_rebuild = true; rgp->ClearPendingBuild(); }
	UI::EditorEventBus::Get().TickFireGraphModified();
}

void EditorRenderPipeline::OnRender()
{
	//   Update swapchain resources each frame (swapchain rotates images)
	if (auto* bb = graph.GetRetainedResource<RHI::TextureDesc, RHI::Texture>("BackBuffer"))
		bb->UpdateRetainedPtr(window->GetViewport()->GetCurrentBackBufferRTV());
	if (auto* ds = graph.GetRetainedResource<RHI::TextureDesc, RHI::Texture>("DepthStencil"))
		ds->UpdateRetainedPtr(window->GetViewport()->GetCurrentBackBufferDSV());
	graph.Execute();
}

EditorRenderPipeline::EditorRenderPipeline(MXRender::Application::Window* in_window)
{
	// window will be set in OnInit
}

MXRender::Application::Window* EditorRenderPipeline::GetWindow()
{
	return window;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

