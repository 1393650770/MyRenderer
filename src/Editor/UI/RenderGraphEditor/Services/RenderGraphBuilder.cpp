// --   Full Builder implementation
// Creates RHI resources from RDGResourceDef, declares pass topology,
// and wires external bindings (swapchain images) into the runtime graph.

#include "UI/RenderGraphEditor/Services/RenderGraphBuilder.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "Render/Core/RenderGraph.h"
#include "Render/Core/RenderGraphResource.h"
#include "RHI/RenderRource.h"
#include "RHI/RenderEnum.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
#include "RHI/RenderCommandList.h"
#include "UI/RenderGraphEditor/Services/PassRegistry.h"
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

String RenderGraphBuilder::s_last_error;

static RHI::TextureDesc BuildTextureDesc(const Render::RDGResourceDef& rd)
{
	RHI::TextureDesc desc;
	desc.format = static_cast<ENUM_TEXTURE_FORMAT>(rd.texture_format);
	desc.width = rd.width;
	desc.height = rd.height;
	desc.mip_level = rd.mip_level;
	desc.layer_count = rd.layer_count;
	desc.samples = rd.samples;
	desc.depth = rd.depth;
	desc.type = static_cast<ENUM_TEXTURE_TYPE>(rd.texture_type);
	desc.usage = static_cast<ENUM_TEXTURE_USAGE_TYPE>(rd.usage);
	desc.resource_state = ENUM_RESOURCE_STATE::Undefined;
	return desc;
}

static RHI::BufferDesc BuildBufferDesc(const Render::RDGResourceDef& rd)
{
	RHI::BufferDesc desc;
	desc.size = rd.buffer_size;
	desc.stride = rd.buffer_stride;
	desc.type = static_cast<ENUM_BUFFER_TYPE>(rd.buffer_type);
	return desc;
}

struct MinimalPassData : public Render::RenderGraphPassDataBase
{
	void Release() override {}
};

Bool RenderGraphBuilder::BuildRuntimeGraph(
	CONST Render::RenderGraphDefinition& def,
	Render::RenderGraph* out_graph,
	CONST Vector<ExternalResourceBinding>& externals)
{
	s_last_error.clear();
	if (!out_graph) { s_last_error = "BuildRuntimeGraph: out_graph is null"; return false; }
	out_graph->Release();

	// ---- Phase 1: Register resources ----
	Map<String, Render::RenderGraphResourceBase*> resource_map;

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
		if (rd.lifetime == Render::RDGResourceLifetime::External || !rd.is_transient)
		{
			if (rd.kind == Render::RDGResourceKind::Buffer)
			{
				auto desc = BuildBufferDesc(rd);
				resource_map[rd.name] = out_graph->AddRetainedResource<RHI::BufferDesc, RHI::Buffer>(rd.name, desc, nullptr);
			}
			else
			{
				auto desc = BuildTextureDesc(rd);
				resource_map[rd.name] = out_graph->AddRetainedResource<RHI::TextureDesc, RHI::Texture>(rd.name, desc, nullptr);
			}
		}
	}

	// ---- Phase 2: Build passes ----
	UInt32 built_count = 0;
	for (auto& pd : def.passes)
	{
		const PassRegistryEntry* reg = PassRegistry::Get().Find(pd.name);
		String pass_name = pd.name;

		// Capture copies for execute lambda
		Vector<String> write_names = pd.write_resources;
		Vector<String> read_names = pd.read_resources;
		Map<String, Render::RenderGraphResourceBase*> res_copy = resource_map;

		auto* pass = out_graph->AddRenderPass<MinimalPassData>(
			pass_name,
			[&](MinimalPassData& data, Render::RenderGraphPassBuilder& builder, RHI::CommandList*)
			{
				for (auto& rn : pd.read_resources) {
					auto it = resource_map.find(rn);
					if (it != resource_map.end()) builder.Read(it->second);
				}
				for (auto& rn : pd.write_resources) {
					auto it = resource_map.find(rn);
					if (it != resource_map.end()) builder.Write(it->second);
				}
			},
			[write_names, read_names, res_copy](const MinimalPassData& data, RHI::CommandList* cmd)
			{
				// Default execute: clear bound render targets with a solid color
				for (auto& rn : write_names) {
					auto it = res_copy.find(rn);
					if (it == res_copy.end() || !it->second) continue;
					if (auto* tex = it->second->GetAsTexture()) {
						Vector<RHI::Texture*> rtvs = { tex };
						RHI::ClearValue cv{ 0.15f, 0.15f, 0.25f, 1.0f };
						Vector<RHI::ClearValue> cvs = { cv };
						cmd->SetRenderTarget(rtvs, nullptr, cvs, false);
						cmd->ClearTexture(tex);
					}
				}
			}
		);

		if (static_cast<UInt32>(pd.pass_flags) & static_cast<UInt32>(Render::RDGPassFlags::NeverCull))
			pass->SetIsCullable(false);
		built_count++;
	}

	std::cout << "[RenderGraphBuilder] Built graph: " << def.graph_name
		<< " (" << built_count << " passes, " << resource_map.size() << " resources)" << std::endl;
	return true;
}

void RenderGraphBuilder::GetBuildStats(
	CONST Render::RenderGraphDefinition& def,
	UInt32& out_pass_count,
	UInt32& out_resource_count)
{
	out_pass_count = (UInt32)def.passes.size();
	out_resource_count = (UInt32)def.resources.size();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
