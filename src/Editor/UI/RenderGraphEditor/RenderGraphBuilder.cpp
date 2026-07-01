#include "RenderGraphBuilder.h"
#include "Render/Core/RenderGraphDefinition.h"
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

String RenderGraphBuilder::s_last_error;

Bool RenderGraphBuilder::BuildRuntimeGraph(
	CONST Render::RenderGraphDefinition& def,
	Render::RenderGraph* out_graph)
{
	s_last_error.clear();

	if (!out_graph)
	{
		s_last_error = "BuildRuntimeGraph: out_graph is null";
		return false;
	}

	// Stub implementation: log the definition that would be built.
	// Full implementation requires mapping RDGPassDef/RDGResourceDef to the
	// actual RenderGraph::AddRenderPass / ::AddRetainedResource API with
	// correct RHI descriptor types (TextureDesc, BufferDesc).
	std::cout << "[RenderGraphBuilder] Building graph: " << def.graph_name << std::endl;
	std::cout << "  Passes: " << def.passes.size() << std::endl;
	std::cout << "  Resources: " << def.resources.size() << std::endl;

	for (auto& pd : def.passes)
	{
		std::cout << "  Pass: " << pd.name
			<< " reads=" << pd.read_resources.size()
			<< " writes=" << pd.write_resources.size()
			<< " creates=" << pd.create_resources.size() << std::endl;
	}

	s_last_error = "BuildRuntimeGraph: stub — topology logged, real build requires RHI descriptor mapping";
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
