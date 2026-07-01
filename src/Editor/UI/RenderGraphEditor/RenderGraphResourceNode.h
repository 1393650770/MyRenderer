#pragma once
#ifndef _RENDERGRAPHRESOURCENODE_
#define _RENDERGRAPHRESOURCENODE_

#include "Core/ConstDefine.h"
#include "UI/BaseNode.h"
#include "RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)
class RenderGraphResourceBase;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(UI)

// Specialized node representing a RenderGraph Resource (Texture or Buffer).
// Used for both editing (defining resource properties) and visualization.
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphResourceNode, public BaseNode)

#pragma region METHOD
public:
	VIRTUAL ~RenderGraphResourceNode() MYDEFAULT;
	RenderGraphResourceNode() MYDEFAULT;
	RenderGraphResourceNode(CONST String& in_name, ResourceNodeType in_res_type = ResourceNodeType::Texture, Bool in_show = true);
	RenderGraphResourceNode(CONST RenderGraphResourceNode& other) MYDELETE;
	RenderGraphResourceNode(RenderGraphResourceNode&& other) MYDELETE;
	RenderGraphResourceNode& operator=(CONST RenderGraphResourceNode& other) MYDELETE;
	RenderGraphResourceNode& operator=(RenderGraphResourceNode&& other) MYDELETE;

	VIRTUAL void METHOD(Draw)() OVERRIDE;
	VIRTUAL void METHOD(Release)() OVERRIDE;
	VIRTUAL BaseNode* METHOD(AsNode)() OVERRIDE { return this; }

	// Pin management with access semantics
	BasePin* METHOD(AddInputPin)(CONST String& in_name, PinAccess access = PinAccess::Write);
	BasePin* METHOD(AddOutputPin)(CONST String& in_name, PinAccess access = PinAccess::Read);

	// Runtime binding
	void METHOD(BindResource)(Render::RenderGraphResourceBase* res);
	Render::RenderGraphResourceBase* METHOD(GetBoundResource)() CONST { return bound_resource; }

	// Resource properties (editable)
	ResourceNodeType METHOD(GetResourceType)() CONST { return resource_type; }
	void METHOD(SetResourceType)(ResourceNodeType type) { resource_type = type; }

	Bool METHOD(GetIsTransient)() CONST { return is_transient; }
	void METHOD(SetIsTransient)(Bool t) { is_transient = t; }

	// Texture-specific properties
	UInt32 METHOD(GetTextureWidth)() CONST { return texture_width; }
	void METHOD(SetTextureWidth)(UInt32 w) { texture_width = w; }
	UInt32 METHOD(GetTextureHeight)() CONST { return texture_height; }
	void METHOD(SetTextureHeight)(UInt32 h) { texture_height = h; }
	UInt8 METHOD(GetMipLevel)() CONST { return mip_level; }
	void METHOD(SetMipLevel)(UInt8 m) { mip_level = m; }
	UInt8 METHOD(GetSamples)() CONST { return samples; }
	void METHOD(SetSamples)(UInt8 s) { samples = s; }
	Int METHOD(GetTextureFormat)() CONST { return texture_format; }
	void METHOD(SetTextureFormat)(Int f) { texture_format = f; }

	// Buffer-specific properties
	UInt64 METHOD(GetBufferSize)() CONST { return buffer_size; }
	void METHOD(SetBufferSize)(UInt64 s) { buffer_size = s; }
	UInt32 METHOD(GetBufferStride)() CONST { return buffer_stride; }
	void METHOD(SetBufferStride)(UInt32 s) { buffer_stride = s; }

	// Lifetime info (set after compile)
	Int METHOD(GetRealizeStep)() CONST { return realize_step; }
	void METHOD(SetRealizeStep)(Int s) { realize_step = s; }
	Int METHOD(GetDerealizeStep)() CONST { return derealize_step; }
	void METHOD(SetDerealizeStep)(Int s) { derealize_step = s; }

protected:
	VIRTUAL void METHOD(RecalcSize)() OVERRIDE;
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	ResourceNodeType resource_type = ResourceNodeType::Texture;
	Render::RenderGraphResourceBase* bound_resource = nullptr; // Weak reference
	Bool is_transient = true;

	// Texture properties
	UInt32 texture_width = 1920;
	UInt32 texture_height = 1080;
	UInt8 mip_level = 1;
	UInt8 samples = 1;
	Int texture_format = 0; // TextureFomat enum value

	// Buffer properties
	UInt64 buffer_size = 256;
	UInt32 buffer_stride = 16;

	// Lifetime
	Int realize_step = -1;
	Int derealize_step = -1;
	UInt64 aliasing_offset = 0;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHRESOURCENODE_
