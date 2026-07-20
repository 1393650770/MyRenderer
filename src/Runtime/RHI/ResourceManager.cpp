#include "ResourceManager.h"
#include "RenderRHI.h"
#include "Render/Core/RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

ResourceManager* g_resource_manager = nullptr;

ResourceManager::ResourceManager() {}

TextureHandle ResourceManager::CreateTexture(CONST TextureDesc& desc, CONST String& debug_name)
{
	Texture* tex = g_render_rhi->CreateTexture(desc);
	if (!tex) return TextureHandle{};
	GenericHandle h = texture_registry.Allocate(tex, debug_name);
	return TextureHandle{ h };
}

BufferHandle ResourceManager::CreateBuffer(CONST BufferDesc& desc, CONST String& debug_name)
{
	Buffer* buf = g_render_rhi->CreateBuffer(desc);
	if (!buf) return BufferHandle{};
	GenericHandle h = buffer_registry.Allocate(buf, debug_name);
	return BufferHandle{ h };
}

PSOHandle ResourceManager::CreatePipelineState(CONST RenderGraphiPipelineStateDesc& desc, CONST String& debug_name)
{
	RenderPipelineState* pso = g_render_rhi->CreateRenderPipelineState(desc);
	if (!pso) return PSOHandle{};
	GenericHandle h = pso_registry.Allocate(pso, debug_name);
	return PSOHandle{ h };
}

TextureHandle ResourceManager::ImportTexture(Texture* existing, CONST String& debug_name)
{
	if (!existing) return TextureHandle{};
	GenericHandle h = texture_registry.Allocate(existing, debug_name);
	return TextureHandle{ h };
}

BufferHandle ResourceManager::ImportBuffer(Buffer* existing, CONST String& debug_name)
{
	if (!existing) return BufferHandle{};
	GenericHandle h = buffer_registry.Allocate(existing, debug_name);
	return BufferHandle{ h };
}

Texture* ResourceManager::Resolve(TextureHandle h) CONST
	{ return texture_registry.Resolve(h.value); }
Buffer* ResourceManager::Resolve(BufferHandle h) CONST
	{ return buffer_registry.Resolve(h.value); }
RenderPipelineState* ResourceManager::Resolve(PSOHandle h) CONST
	{ return pso_registry.Resolve(h.value); }

void ResourceManager::DestroyTexture(TextureHandle h)
{
	if (!h.IsValid()) return;
	Texture* tex = texture_registry.Free(h.value);
	if (!tex) return;
	MXRender::Render::PushDeferredDestruction(UniquePtr<RenderResource>(tex));
}

void ResourceManager::DestroyBuffer(BufferHandle h)
{
	if (!h.IsValid()) return;
	Buffer* buf = buffer_registry.Free(h.value);
	if (!buf) return;
	MXRender::Render::PushDeferredDestruction(UniquePtr<RenderResource>(buf));
}

void ResourceManager::DestroyPipelineState(PSOHandle h)
{
	if (!h.IsValid()) return;
	pso_registry.Free(h.value);
}

void ResourceManager::ProcessPendingDestruction() {}

// Handle resolve wrappers (hide g_resource_manager from callers)
Texture* Resolve(TextureHandle h) { return g_resource_manager ? g_resource_manager->Resolve(h) : nullptr; }
Buffer* Resolve(BufferHandle h)   { return g_resource_manager ? g_resource_manager->Resolve(h) : nullptr; }
RenderPipelineState* Resolve(PSOHandle h) { return g_resource_manager ? g_resource_manager->Resolve(h) : nullptr; }

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
