#pragma once
#ifndef _RESOURCEMANAGER_
#define _RESOURCEMANAGER_

#include "Core/ConstDefine.h"
#include "Core/ResourceHandle.h"
#include "Core/ResourceRegistry.h"
#include "RHIHandleTypes.h"
#include "RenderRource.h"
#include "RenderTexture.h"
#include "RenderBuffer.h"
#include "RenderShader.h"
#include "RenderPipelineState.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(ResourceManager, public RenderResource)
#pragma region METHOD
public:
	ResourceManager();
	VIRTUAL ~ResourceManager() MYDEFAULT;

	TextureHandle METHOD(CreateTexture)(CONST TextureDesc& desc, CONST String& debug_name);
	BufferHandle  METHOD(CreateBuffer)(CONST BufferDesc& desc, CONST String& debug_name);
	PSOHandle     METHOD(CreatePipelineState)(CONST RenderGraphiPipelineStateDesc& desc, CONST String& debug_name);
	TextureHandle METHOD(ImportTexture)(Texture* existing, CONST String& debug_name);
	BufferHandle  METHOD(ImportBuffer)(Buffer* existing, CONST String& debug_name);

	Texture*             METHOD(Resolve)(TextureHandle h) CONST;
	Buffer*              METHOD(Resolve)(BufferHandle h) CONST;
	RenderPipelineState* METHOD(Resolve)(PSOHandle h) CONST;

	void METHOD(DestroyTexture)(TextureHandle h);
	void METHOD(DestroyBuffer)(BufferHandle h);
	void METHOD(DestroyPipelineState)(PSOHandle h);

	void METHOD(ProcessPendingDestruction)();

	ResourceRegistry<Texture>&            METHOD(GetTextureRegistry)() { return texture_registry; }
	ResourceRegistry<Buffer>&             METHOD(GetBufferRegistry)()  { return buffer_registry; }
	ResourceRegistry<RenderPipelineState>& METHOD(GetPSORegistry)()   { return pso_registry; }

protected:
	void METHOD(EnqueueDeferredFree)(GenericHandle handle);
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	ResourceRegistry<Texture>              texture_registry;
	ResourceRegistry<Buffer>               buffer_registry;
	ResourceRegistry<RenderPipelineState>  pso_registry;

	Vector<GenericHandle> deferred_free_queue;
	Vector<GenericHandle> pending_free_next;
	Vector<GenericHandle> pending_free_now;
private:
#pragma endregion
MYRENDERER_END_CLASS

extern ResourceManager* g_resource_manager;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
