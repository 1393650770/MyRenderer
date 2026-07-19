#include "SampleApp.h"
#include <iostream>
#include <cstdlib>
#include "Application/Window.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderBuffer.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "Render/Core/RenderGraphSerializer.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

Int SampleApp::RunSample(SampleApp& in_app, CONST String& in_title)
{
	Window window(in_title);
	window.InitWindow();
	window.Run(&in_app);
	system("pause");
	return 0;
}

void SampleApp::OnInit_Logic(Application::Window* in_window)
{
	window = in_window;

	// Register the swapchain backbuffer (+ depth) as retained RDG resources
	RHI::Texture* backbuffer_rtv = window->GetViewport()->GetCurrentBackBufferRTV();
	RHI::Texture* backbuffer_dsv = window->GetViewport()->GetCurrentBackBufferDSV();

	backbuffer_resource = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
		"BackBuffer", backbuffer_rtv->GetTextureDesc(), backbuffer_rtv);
	if (backbuffer_dsv)
	{
		depth_stencil_resource = graph.AddRetainedResource<RHI::TextureDesc, RHI::Texture>(
			"DepthStencil", backbuffer_dsv->GetTextureDesc(), backbuffer_dsv);
	}

	OnInitScene();

	if (auto_compile)
		graph.Compile();
}

void SampleApp::OnShutdown_Logic()
{
	OnShutdownScene();
	graph.Release();
}

void SampleApp::OnRender()
{
	graph.Execute();
}

RHI::Texture* SampleApp::GetBackBuffer() CONST
{
	return window->GetViewport()->GetCurrentBackBufferRTV();
}

RHI::Texture* SampleApp::GetDepthStencil() CONST
{
	return window->GetViewport()->GetCurrentBackBufferDSV();
}

void SampleApp::BindBackBufferTarget(RHI::CommandList* in_cmd) CONST
{
	Vector<RHI::Texture*> rtvs = { GetBackBuffer() };
	RHI::Texture* dsv = GetDepthStencil();
	Vector<RHI::ClearValue> clear_values;
	for (auto* rtv : rtvs)
		clear_values.push_back(rtv->GetTextureDesc().clear_value);
	if (dsv)
		clear_values.push_back(dsv->GetTextureDesc().clear_value);
	in_cmd->SetRenderTarget(rtvs, dsv, clear_values, dsv != nullptr);
}

void SampleApp::SaveGraphDefinition(CONST String& in_graph_name, CONST String& in_file_path)
{
	Render::RenderGraphDefinition def;
	def.graph_name = in_graph_name;
	def.version = 2;

	for (auto& res : graph.GetResources())
	{
		Render::RDGResourceDef rd;
		rd.name = res->GetName();
		rd.is_transient = res->GetIsTransient();
		if (auto* tex = res->GetAsTexture())
		{
			rd.desc = tex->GetTextureDesc();
		}
		else if (res->GetAsBuffer())
		{
			rd.desc = res->GetAsBuffer()->GetBufferDesc();
		}
		def.resources.push_back(rd);
	}

	for (auto& pass : graph.GetPasses())
	{
		Render::RDGPassDef pd;
		pd.name = pass->GetName();
		pd.pass_kind = Render::RDGPassKind::Graphics;
		pd.shader_path = pass->GetShaderPath();
		pd.vertex_count = pass->GetVertexCount();
		for (auto* r : pass->GetReadResources())  pd.read_resources.push_back(r->GetName());
		for (auto* w : pass->GetWriteResources()) pd.write_resources.push_back(w->GetName());
		for (auto* c : pass->GetCreateResources()) pd.create_resources.push_back(c->GetName());
		def.passes.push_back(pd);
	}

	Render::RenderGraphSerializer::SaveGraph(def, in_file_path);
	std::cout << "[SampleApp] Graph saved to " << in_file_path << std::endl;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
