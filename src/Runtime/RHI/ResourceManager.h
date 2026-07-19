#pragma once
#ifndef _RESOURCEMANAGER_
#define _RESOURCEMANAGER_

#include "Core/ConstDefine.h"
#include "Core/ResourceHandle.h"
#include "Core/ResourceRegistry.h"
#include "RenderRource.h"
#include "RenderTexture.h"
#include "RenderBuffer.h"
#include "RenderShader.h"
#include "RenderPipelineState.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

// --  RHI  --  Handle Tag  --  --  --  --  --   --  --  --  compile-time  --  --  --  --
struct TagTexture {};
struct TagBuffer {};
struct TagShader {};
struct TagPipelineState {};
struct TagSRB {};
struct TagSampler {};
using TextureHandle = ResourceHandle<TagTexture>;
using BufferHandle  = ResourceHandle<TagBuffer>;
using ShaderHandle  = ResourceHandle<TagShader>;
using PSOHandle     = ResourceHandle<TagPipelineState>;
using SRBHandle     = ResourceHandle<TagSRB>;
using SamplerHandle = ResourceHandle<TagSampler>;

// --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
//  --  --  Handle  --  --  --  --  --  --  --  --  --  --  --  --  --
//  --  --  create/resolve/destroy  --  --  --  --  --  --  --  --  --
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(ResourceManager, public RenderResource)
#pragma region METHOD
public:
	ResourceManager();
	VIRTUAL ~ResourceManager() MYDEFAULT;

	// --  --  --  --  --  --  --  --  Handle  --  --  --
	TextureHandle METHOD(CreateTexture)(CONST TextureDesc& desc, CONST String& debug_name);
	BufferHandle  METHOD(CreateBuffer)(CONST BufferDesc& desc, CONST String& debug_name);

	// --  --  --  --  --  --  --  --   stale handle  --  --  nullptr  --  --
	Texture* METHOD(Resolve)(TextureHandle h) CONST;
	Buffer*  METHOD(Resolve)(BufferHandle h) CONST;

	// --  --  --  --  --  --   GPU  --  --  --  --  --  --  --
	void METHOD(DestroyTexture)(TextureHandle h);
	void METHOD(DestroyBuffer)(BufferHandle h);

	// --  --  --  --  --  --  --  Execute)  --  --  --  --  --  --
	void METHOD(ProcessPendingDestruction)();

	// --  --  --  --  --  Registry（VK  --  --  --  --  --  --）
	ResourceRegistry<Texture>& METHOD(GetTextureRegistry)() { return texture_registry; }
	ResourceRegistry<Buffer>&  METHOD(GetBufferRegistry)()  { return buffer_registry; }

protected:
	void METHOD(EnqueueDeferredFree)(GenericHandle handle);
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	ResourceRegistry<Texture> texture_registry;
	ResourceRegistry<Buffer>  buffer_registry;

	//  --  --  --  --  --  --  --  --  --  --  --  --
	Vector<GenericHandle> deferred_free_queue;   //  --  --  --  --  --  --
	Vector<GenericHandle> pending_free_next;     // 1  --  --  --
	Vector<GenericHandle> pending_free_now;      // 2  --  --  --  --  --  --  --  --
private:
#pragma endregion
MYRENDERER_END_CLASS

// --  --  --  --  --  --  --  --  --  --
extern ResourceManager* g_resource_manager;

MYRENDERER_END_NAMESPACE  // RHI
MYRENDERER_END_NAMESPACE  // MXRender
#endif
