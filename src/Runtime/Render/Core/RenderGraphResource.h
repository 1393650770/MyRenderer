#pragma once
#ifndef _RENDERGRAPHRESOURCE_
#define _RENDERGRAPHRESOURCE_
#include "RenderGraphResourceImplementation.h"
#include <variant>
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


class RenderGraphPassBase;

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

