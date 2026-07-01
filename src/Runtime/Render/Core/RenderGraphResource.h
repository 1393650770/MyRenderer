#pragma once
#ifndef _RENDERGRAPHRESOURCE_
#define _RENDERGRAPHRESOURCE_
#include "RenderGraphResourceImplementation.h"
#include <variant>
#include <type_traits>
#include "RHI/RenderEnum.h"
#include "RHI/RenderTexture.h"
#include "RHI/RenderBuffer.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Global deferred destruction queue for transient RDG resources.
void PushDeferredDestruction(std::unique_ptr<MXRender::RHI::RenderResource>&& resource);
void ProcessDeferredDestruction();

// ---------------------------------------------------------------------------
// Cross-frame resource pooling bridge.
// Declared here (Render layer) so Derealize/Realize can use them.
// Implemented in the VK layer (VK_ResourcePool.cpp).
// ---------------------------------------------------------------------------
std::unique_ptr<MXRender::RHI::Texture> AcquirePooledTexture(CONST MXRender::RHI::TextureDesc& desc);
void ReturnPooledTexture(std::unique_ptr<MXRender::RHI::Texture> texture, CONST MXRender::RHI::TextureDesc& desc);
std::unique_ptr<MXRender::RHI::Buffer> AcquirePooledBuffer(CONST MXRender::RHI::BufferDesc& desc);
void ReturnPooledBuffer(std::unique_ptr<MXRender::RHI::Buffer> buffer, CONST MXRender::RHI::BufferDesc& desc);

// Global resource pool pointer — defined in VK_ResourcePool.cpp, set in VK_Device::Init().
// Debug name bridge for RenderDoc/validation layers.
// Sets VK_EXT_debug_utils object name on the underlying Vulkan resource.
// Declared here (Render layer) so Execute() can set names.
// Implemented in the VK layer (VK_ResourcePool.cpp).
void SetDebugNameForRHIResource(MXRender::RHI::RenderResource* resource, CONST String& name);

extern void* g_resource_pool; // Opaque pointer; actual type is VK_ResourcePool* in ::MXRender::RHI::Vulkan

class RenderGraphPassBase;

// Describes the access pattern of a pass on a resource.
struct PassResourceAccess
{
	CONST RenderGraphPassBase* pass = nullptr;
	MXRender::ENUM_RESOURCE_STATE required_state = MXRender::ENUM_RESOURCE_STATE::Undefined;
	bool is_write = false;
		UInt64 modification_stamp = 0;
};

	struct RHIBarrierDesc
	{
		class RenderGraphResourceBase* resource = nullptr;
		ENUM_RESOURCE_STATE src_state = ENUM_RESOURCE_STATE::Undefined;
		ENUM_RESOURCE_STATE dst_state = ENUM_RESOURCE_STATE::Undefined;
		Bool is_prologue = true;
	};

MYRENDERER_BEGIN_CLASS(RenderGraphResourceBase)

#pragma region MATHOD

public:
	explicit RenderGraphResourceBase(CONST String& name, CONST RenderGraphPassBase* creator) : name(name), create_pass(creator), ref_count(0)
	{
		static UInt32 static_id = 0;
		id = static_id++;
	}
	RenderGraphResourceBase() MYDEFAULT;
	VIRTUAL ~RenderGraphResourceBase() MYDEFAULT;
	RenderGraphResourceBase(RenderGraphResourceBase&& temp) MYDEFAULT;
	RenderGraphResourceBase& operator=(CONST RenderGraphResourceBase& that) MYDELETE;
	RenderGraphResourceBase& operator=(RenderGraphResourceBase&& temp) MYDEFAULT;

	UInt32 METHOD(GetID)() CONST;
	CONST String& METHOD(GetName)() CONST;
	void METHOD(SetName)(CONST String& in_name);
	Bool METHOD(GetIsTransient)() CONST;

	VIRTUAL bool IsTextureResource() CONST { return false; }
	VIRTUAL bool IsBufferResource() CONST { return false; }

	// Get the actual RHI resource pointers. Returns nullptr if not applicable.
	VIRTUAL MXRender::RHI::Texture* GetAsTexture() CONST { return nullptr; }
	VIRTUAL MXRender::RHI::Buffer* GetAsBuffer() CONST { return nullptr; }

	// Track the current resource state for automatic barrier generation.
	MXRender::ENUM_RESOURCE_STATE tracked_state = MXRender::ENUM_RESOURCE_STATE::Undefined;
	// Access sequence of passes on this resource (ordered by pass execution).
	Vector<PassResourceAccess> access_sequence;

protected:
	VIRTUAL void METHOD(Realize)() PURE;
	VIRTUAL void METHOD(Derealize)() PURE;
private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	friend class RenderGraph;
	friend class RenderGraphPassBuilder;

	UInt32                          id;
	String                         name;
	CONST RenderGraphPassBase* create_pass;
	Vector<CONST RenderGraphPassBase*> read_passes;
	Vector<CONST RenderGraphPassBase*> write_passes;
	UInt32                          ref_count;
private:


#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_TEMPLATE_HEAD(typename description_type_, typename actual_type_)
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphResource, public RenderGraphResourceBase)

#pragma region MATHOD
public:
	using description_type = description_type_;
	using actual_type = actual_type_;

	explicit RenderGraphResource(CONST String& name, CONST RenderGraphPassBase* creator, CONST description_type& description)
		: RenderGraphResourceBase(name, creator), description(description), actual(std::unique_ptr<actual_type>())
	{
		// Transient (normal) constructor.
	}
	explicit RenderGraphResource(CONST String& name, CONST description_type& in_description, actual_type* in_actual = nullptr)
		: RenderGraphResourceBase(name, nullptr), description(in_description), actual(in_actual)
	{
		// Retained (import) constructor.
		if (!in_actual)
			actual = RealizeResource<description_type, actual_type>(in_description);
	}
	RenderGraphResource(CONST RenderGraphResource& that) MYDELETE;
	RenderGraphResource(RenderGraphResource&& temp) MYDEFAULT;
	~RenderGraphResource() MYDEFAULT;
	RenderGraphResource& operator=(CONST RenderGraphResource& that) MYDELETE;
	RenderGraphResource& operator=(RenderGraphResource&& temp) MYDEFAULT;

	CONST description_type& METHOD(GetDescription)() CONST
	{
		return description;
	}
	actual_type* METHOD(GetActual)() CONST // If transient, only valid through the realized interval of the resource.
	{
		return std::holds_alternative<std::unique_ptr<actual_type>>(actual) ? std::get<std::unique_ptr<actual_type>>(actual).get() : std::get<actual_type*>(actual);
	}

	// Override type-specific accessors.
	VIRTUAL MXRender::RHI::Texture* GetAsTexture() CONST override
	{
		if constexpr (std::is_same<actual_type, MXRender::RHI::Texture>::value)
			return std::holds_alternative<std::unique_ptr<actual_type>>(actual) ? std::get<std::unique_ptr<actual_type>>(actual).get() : std::get<actual_type*>(actual);
		return nullptr;
	}
	VIRTUAL MXRender::RHI::Buffer* GetAsBuffer() CONST override
	{
		if constexpr (std::is_same<actual_type, MXRender::RHI::Buffer>::value)
			return std::holds_alternative<std::unique_ptr<actual_type>>(actual) ? std::get<std::unique_ptr<actual_type>>(actual).get() : std::get<actual_type*>(actual);
		return nullptr;
	}

protected:
	VIRTUAL void METHOD(Realize)() override
	{
		if (GetIsTransient())
		{
			std::get<std::unique_ptr<actual_type>>(actual) = RealizeResource<description_type, actual_type>(description);
		}
	}
	VIRTUAL void METHOD(Derealize)() override
	{
		if (GetIsTransient())
		{
			std::get<std::unique_ptr<actual_type>>(actual).reset();
		}
	}
private:

#pragma endregion


#pragma region MEMBER
public:

protected:
	description_type                                         description ;
	std::variant<std::unique_ptr<actual_type>, actual_type*> actual ;
private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
