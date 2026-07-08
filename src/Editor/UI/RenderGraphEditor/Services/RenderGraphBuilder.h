#pragma once
#ifndef _RENDERGRAPHBUILDER_
#define _RENDERGRAPHBUILDER_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)
class RenderGraph;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(UI)

// Maps a resource name to an actual GPU resource (e.g. swapchain images).
// Use void* to avoid pulling RHI headers into every consumer.
struct ExternalResourceBinding
{
	String resource_name;
	void* texture = nullptr; // RHI::Texture*
	void* buffer  = nullptr; // RHI::Buffer*
};

// Builds a runtime RenderGraph from a RenderGraphDefinition.
// This bridges the editor's data representation to the runtime execution graph.
MYRENDERER_BEGIN_CLASS(RenderGraphBuilder)

#pragma region METHOD
public:
	// Build a runtime graph. External bindings override resource creation
	// (used for swapchain BackBuffer / DepthStencil).
	static Bool METHOD(BuildRuntimeGraph)(
		CONST Render::RenderGraphDefinition& def,
		Render::RenderGraph* out_graph,
		CONST Vector<ExternalResourceBinding>& externals = {});

	// Get the count of passes/resources that would be built (for preview).
	static void METHOD(GetBuildStats)(
		CONST Render::RenderGraphDefinition& def,
		UInt32& out_pass_count,
		UInt32& out_resource_count);

private:
	static String s_last_error;

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHBUILDER_
