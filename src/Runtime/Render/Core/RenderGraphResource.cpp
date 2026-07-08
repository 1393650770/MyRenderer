#include "RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

static constexpr UInt32 DEFERRED_FRAME_DEPTH = 3;

struct DeferredItem
{
	std::unique_ptr<MXRender::RHI::RenderResource> resource;
	UInt64 frame_number;
};

static Vector<DeferredItem> g_deferred_queue;
static UInt64 g_deferred_frame_counter = 0;

void PushDeferredDestruction(std::unique_ptr<MXRender::RHI::RenderResource>&& resource)
{
	if (!resource) return;
	g_deferred_queue.push_back({std::move(resource), g_deferred_frame_counter});
}

void ProcessDeferredDestruction()
{
	g_deferred_frame_counter++;
	UInt64 safe_boundary = (g_deferred_frame_counter > DEFERRED_FRAME_DEPTH)
		? (g_deferred_frame_counter - DEFERRED_FRAME_DEPTH) : 0;

	for (Int i = (Int)g_deferred_queue.size() - 1; i >= 0; --i)
	{
		if (g_deferred_queue[i].frame_number <= safe_boundary)
		{
			g_deferred_queue[i] = std::move(g_deferred_queue.back());
			g_deferred_queue.pop_back();
		}
	}
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
