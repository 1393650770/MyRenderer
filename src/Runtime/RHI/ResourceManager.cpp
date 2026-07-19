#include "ResourceManager.h"
#include "RenderRHI.h"
#include "Render/Core/RenderGraphResource.h"  // for PushDeferredDestruction

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

ResourceManager* g_resource_manager = nullptr;

ResourceManager::ResourceManager()
{
}

TextureHandle ResourceManager::CreateTexture(CONST TextureDesc& desc, CONST String& debug_name)
{
	Texture* tex = RHICreateTexture(desc);
	if (!tex) return TextureHandle{};

	GenericHandle h = texture_registry.Allocate(tex, debug_name);
	return TextureHandle{ h };
}

BufferHandle ResourceManager::CreateBuffer(CONST BufferDesc& desc, CONST String& debug_name)
{
	Buffer* buf = RHICreateBuffer(desc);
	if (!buf) return BufferHandle{};

	GenericHandle h = buffer_registry.Allocate(buf, debug_name);
	return BufferHandle{ h };
}

Texture* ResourceManager::Resolve(TextureHandle h) CONST
{
	return texture_registry.Resolve(h.value);
}

Buffer* ResourceManager::Resolve(BufferHandle h) CONST
{
	return buffer_registry.Resolve(h.value);
}

void ResourceManager::DestroyTexture(TextureHandle h)
{
	if (!h.IsValid()) return;
	// Free() returns the owned pointer, bumps generation
	Texture* tex = texture_registry.Free(h.value);
	if (!tex) return; // already freed or stale handle

	// Deferred destruction — GPU-safe after 3 frames
	MXRender::Render::PushDeferredDestruction(UniquePtr<RenderResource>(tex));
}

void ResourceManager::DestroyBuffer(BufferHandle h)
{
	if (!h.IsValid()) return;
	Buffer* buf = buffer_registry.Free(h.value);
	if (!buf) return;

	MXRender::Render::PushDeferredDestruction(UniquePtr<RenderResource>(buf));
}

void ResourceManager::ProcessPendingDestruction()
{
	// Deferred destruction is handled by the Render layer's 3-frame FIFO queue
	// (PushDeferredDestruction / ProcessDeferredDestruction).
	// ResourceManager just calls Free() → returns pointer → pushes to that queue.
	// This method is a no-op for now; destruction happens in RenderGraph::Execute().
}

MYRENDERER_END_NAMESPACE  // RHI
MYRENDERER_END_NAMESPACE  // MXRender
