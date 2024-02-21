#pragma once
#ifndef _RENDERGRAPHRESOURCE_
#define _RENDERGRAPHRESOURCE_
#include "RenderGraphResourceImplementation.h"
#include "Core/ConstDefine.h"
#include <variant>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


class RenderGraphPassBase;

MYRENDERER_BEGIN_CLASS(RenderGraphResourceBase)

#pragma region MATHOD

public:
	RenderGraphResourceBase() DEFAULT;
	VIRTUAL ~RenderGraphResourceBase() DEFAULT;
	RenderGraphResourceBase(RenderGraphResourceBase&& temp) DEFAULT;
	RenderGraphResourceBase& operator=(CONST RenderGraphResourceBase& that) DELETE;
	RenderGraphResourceBase& operator=(RenderGraphResourceBase&& temp) DEFAULT;

	UInt32 METHOD(GetID)() CONST;
	CONST String& METHOD(GetName)() CONST;
	void METHOD(SetName)(CONST String& in_name);
	Bool METHOD(GetIsTransient)() CONST;

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
	explicit RenderGraphResource(CONST String& name, CONST description_type& description, actual_type* actual = nullptr)
		: RenderGraphResourceBase(name, nullptr), description(description), actual(actual)
	{
		// Retained (import) constructor.
		if (!actual) 
			actual = Realize<description_type, actual_type>(description);
	}
	RenderGraphResource(CONST RenderGraphResource& that) DELETE;
	RenderGraphResource(RenderGraphResource&& temp) DEFAULT;
	~RenderGraphResource() DEFAULT;
	RenderGraphResource& operator=(CONST RenderGraphResource& that) DELETE;
	RenderGraphResource& operator=(RenderGraphResource&& temp) DEFAULT;

	CONST description_type& METHOD(GetDescription)() CONST
	{
		return description;
	}
	actual_type* METHOD(GetActual)() CONST // If transient, only valid through the realized interval of the resource.
	{
		return std::holds_alternative<std::unique_ptr<actual_type>>(actual) ? std::get<std::unique_ptr<actual_type>>(actual).get() : std::get<actual_type*>(actual);
	}
protected:
	VIRTUAL void METHOD(Realize)() override
	{
		if (GetIsTransient())
		{
			std::get<std::unique_ptr<actual_type>>(actual) = Realize<description_type, actual_type>(description);
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

