#include "RenderGraphResource.h"
#include <mutex>

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
static std::mutex g_deferred_mutex;

void PushDeferredDestruction(std::unique_ptr<MXRender::RHI::RenderResource>&& resource)
{
	if (!resource) return;
	std::lock_guard<std::mutex> lock(g_deferred_mutex);
	g_deferred_queue.push_back({std::move(resource), g_deferred_frame_counter});
}

void ProcessDeferredDestruction()
{
	g_deferred_frame_counter++;
	// Nothing is safe to destroy until DEFERRED_FRAME_DEPTH frames have passed.
	// Without this early-out the clamp below made frame-0 pushes (frame_number==0)
	// eligible immediately (0 <= 0), destroying resources referenced by a command
	// buffer that had not even been submitted yet.
	if (g_deferred_frame_counter <= DEFERRED_FRAME_DEPTH)
		return;
	UInt64 safe_boundary = g_deferred_frame_counter - DEFERRED_FRAME_DEPTH;

	std::lock_guard<std::mutex> lock(g_deferred_mutex);
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
