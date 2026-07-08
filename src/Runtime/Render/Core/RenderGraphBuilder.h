#pragma once
#ifndef _RENDERGRAPHBUILDER_
#define _RENDERGRAPHBUILDER_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)
class RenderGraph;
class RenderGraphResourceBase;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Render)

// Callback type for pass implementations.
// Takes the command list and a map of resource name -> runtime resource.
using PassExecuteFunc = std::function<void(
	MXRender::RHI::CommandList* cmd,
	Map<String, MXRender::Render::RenderGraphResourceBase*>& resource_map)>;

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
		CONST RenderGraphDefinition& def,
		RenderGraph* out_graph,
		CONST Vector<ExternalResourceBinding>& externals = {});

	// Get the count of passes/resources that would be built (for preview).
	static void METHOD(GetBuildStats)(
		CONST RenderGraphDefinition& def,
		UInt32& out_pass_count,
		UInt32& out_resource_count);

	// Register a pass execution callback. When Builder encounters a pass
	// with a matching name, the registered callback is invoked instead of
	// the default clear. Pass "" matches all passes (fallback).
	static void METHOD(RegisterPassExecute)(CONST String& pass_name, PassExecuteFunc func);

private:
	static String s_last_error;

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHBUILDER_
