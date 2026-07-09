#include "Render/Core/RenderGraphBuilder.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "Render/Core/RenderGraph.h"
#include "Render/Core/RenderGraphResource.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderEnum.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderCommandList.h"
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

String RenderGraphBuilder::s_last_error;

// Global pass execution registry
static Map<String, PassExecuteFunc> g_pass_execute_registry;

void RenderGraphBuilder::RegisterPassExecute(CONST String& pass_name, PassExecuteFunc func)
{
	g_pass_execute_registry[pass_name] = std::move(func);
}

struct MinimalPassData : public RenderGraphPassDataBase
{
	PassExecuteFunc exec_callback; // Registered pass execute (nullptr = default clear)
	void Release() override {}
};

Bool RenderGraphBuilder::BuildRuntimeGraph(
	CONST RenderGraphDefinition& def,
	RenderGraph* out_graph,
	CONST Vector<ExternalResourceBinding>& externals)
{
	s_last_error.clear();
	if (!out_graph) { s_last_error = "BuildRuntimeGraph: out_graph is null"; return false; }
	out_graph->Release();

	// ---- Phase 1: Register resources ----
	Map<String, RenderGraphResourceBase*> resource_map;

	// External bindings first (swapchain BackBuffer / DepthStencil)
	for (auto& ext : externals)
	{
		if (ext.texture)
			resource_map[ext.resource_name] = out_graph->RegisterExternalResource<RHI::TextureDesc, RHI::Texture>(
				ext.resource_name, static_cast<RHI::Texture*>(ext.texture));
		else if (ext.buffer)
			resource_map[ext.resource_name] = out_graph->RegisterExternalResource<RHI::BufferDesc, RHI::Buffer>(
				ext.resource_name, static_cast<RHI::Buffer*>(ext.buffer));
	}

	// Defined resources (skip if already bound externally)
	for (auto& rd : def.resources)
	{
		if (resource_map.count(rd.name)) continue;
		if (rd.lifetime == RDGResourceLifetime::External || !rd.is_transient)
		{
			if (std::holds_alternative<RHI::BufferDesc>(rd.desc))
			{
				auto desc = std::get<RHI::BufferDesc>(rd.desc);
				resource_map[rd.name] = out_graph->AddRetainedResource<RHI::BufferDesc, RHI::Buffer>(rd.name, desc, nullptr);
			}
			else if (std::holds_alternative<RHI::ShaderDesc>(rd.desc))
			{
				auto desc = std::get<RHI::ShaderDesc>(rd.desc);
				resource_map[rd.name] = out_graph->AddRetainedResource<RHI::ShaderDesc, RHI::Shader>(rd.name, desc, nullptr);
			}
			else
			{
				auto desc = std::get<RHI::TextureDesc>(rd.desc);
				resource_map[rd.name] = out_graph->AddRetainedResource<RHI::TextureDesc, RHI::Texture>(rd.name, desc, nullptr);
			}
		}
	}

	// ---- Phase 2: Build passes ----
	UInt32 built_count = 0;
	for (auto& pd : def.passes)
	{
		String pass_name = pd.name;

		// Look up registered pass execution callback
		PassExecuteFunc exec_cb = nullptr;
		auto exec_it = g_pass_execute_registry.find(pass_name);
		if (exec_it != g_pass_execute_registry.end())
			exec_cb = exec_it->second;
		else {
			auto wildcard_it = g_pass_execute_registry.find("");
			if (wildcard_it != g_pass_execute_registry.end())
				exec_cb = wildcard_it->second;
		}

		// Capture copies for execute lambda
		Vector<String> write_names = pd.write_resources;
		Map<String, RenderGraphResourceBase*> exec_res_map = resource_map;

		auto* pass = out_graph->AddRenderPass<MinimalPassData>(
			pass_name,
			[&, exec_cb](MinimalPassData& data, RenderGraphPassBuilder& builder, RHI::CommandList*)
			{
				data.exec_callback = exec_cb;
				for (auto& rn : pd.read_resources) {
					auto it = resource_map.find(rn);
					if (it != resource_map.end()) builder.Read(it->second);
				}
				for (auto& rn : pd.write_resources) {
					auto it = resource_map.find(rn);
					if (it != resource_map.end()) builder.Write(it->second);
				}
			},
			[write_names, exec_res_map](const MinimalPassData& data, RHI::CommandList* cmd)
			{
				if (data.exec_callback)
				{
					auto mutable_map = exec_res_map;
					data.exec_callback(cmd, mutable_map);
				}
				else
				{
					// Default execute: clear bound render targets
					for (auto& rn : write_names) {
						auto it = exec_res_map.find(rn);
						if (it == exec_res_map.end() || !it->second) continue;
						if (auto* tex = it->second->GetAsTexture()) {
							Vector<RHI::Texture*> rtvs = { tex };
							RHI::ClearValue cv{ 0.15f, 0.15f, 0.25f, 1.0f };
							Vector<RHI::ClearValue> cvs = { cv };
							cmd->SetRenderTarget(rtvs, nullptr, cvs, false);
							cmd->ClearTexture(tex);
						}
					}
				}
			}
		);

		if (static_cast<UInt32>(pd.pass_flags) & static_cast<UInt32>(RDGPassFlags::NeverCull))
			pass->SetIsCullable(false);
		built_count++;
	}

	std::cout << "[RenderGraphBuilder] Built graph: " << def.graph_name
		<< " (" << built_count << " passes, " << resource_map.size() << " resources)" << std::endl;
	return true;
}

void RenderGraphBuilder::GetBuildStats(
	CONST RenderGraphDefinition& def,
	UInt32& out_pass_count,
	UInt32& out_resource_count)
{
	out_pass_count = (UInt32)def.passes.size();
	out_resource_count = (UInt32)def.resources.size();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
