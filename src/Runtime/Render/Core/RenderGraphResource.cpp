#include "RenderGraphResource.h"
#include <functional>
#include <vector>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Global deferred destruction queue.
// Transient resources are pushed here during RenderGraph::Execute() and
// destroyed at VulkanRHI::RenderEnd(), after the command buffer has been submitted.
static Vector<std::function<void()>> g_deferred_destruction_queue;

void PushDeferredDestruction(std::function<void()>&& deleter)
{
	g_deferred_destruction_queue.push_back(std::move(deleter));
}

void ProcessDeferredDestruction()
{
	for (auto& fn : g_deferred_destruction_queue)
		fn();
	g_deferred_destruction_queue.clear();
}

UInt32 RenderGraphResourceBase::GetID() CONST
{
	return id;
}

CONST String& RenderGraphResourceBase::GetName() CONST
{
	return name;
}

void RenderGraphResourceBase::SetName(CONST String& in_name)
{
	name = in_name;
}

Bool RenderGraphResourceBase::GetIsTransient() CONST
{
	return create_pass != nullptr;
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
