#pragma once
#ifndef _RENDERGRAPHPASS_
#define _RENDERGRAPHPASS_
#include "Core/ConstDefine.h"
#include <functional>


MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_NAMESPACE(RHI)
class CommandList;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Render)

class RenderGraphPassBuilder;
class RenderGraphResourceBase;
class RenderGraph;

MYRENDERER_BEGIN_CLASS(RenderGraphPassBase)

#pragma region MATHOD

public:
	explicit RenderGraphPassBase(CONST std::string& in_name);
	RenderGraphPassBase() DEFAULT;
	VIRTUAL ~RenderGraphPassBase() DEFAULT;
	RenderGraphPassBase(RenderGraphPassBase&& temp) DEFAULT;
	RenderGraphPassBase& operator=(CONST RenderGraphPassBase& that) DELETE;
	RenderGraphPassBase& operator=(RenderGraphPassBase&& temp) DEFAULT;

	CONST String& METHOD(GetName)() CONST;
	void METHOD(SetName)(CONST String& in_name);
	Bool METHOD(GetIsCullable)() CONST;
	void METHOD(SetIsCullable)(Bool in_is_cullable);

protected:
	friend class RenderGraphPassBuilder;
	friend class RenderGraph;

	VIRTUAL void METHOD(SetUp)(RenderGraphPassBuilder& in_builder) PURE;
	VIRTUAL void METHOD(Execute)() PURE;
private:

#pragma endregion

#pragma region MEMBER
public:


protected:
	String pass_name;
	Bool   is_cullable=true;
	Vector<CONST RenderGraphResourceBase*> create_resources;
	Vector<CONST RenderGraphResourceBase*> read_resources;
	Vector<CONST RenderGraphResourceBase*> write_resources;
	UInt8 ref_count=0;
private:

#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_TEMPLATE_HEAD(typename data_type_)
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphPass,public RenderGraphPassBase)
using data_type = data_type_;
#pragma region MATHOD

public:
	explicit RenderGraphPass(
		CONST String& name,
		CONST std::function<void(data_type&, RenderGraphPassBuilder&, MXRender::RHI::CommandList*)>& in_setup,
		CONST std::function<void(CONST data_type&, MXRender::RHI::CommandList*)>& in_execute) : RenderGraphPassBase(name), setup(in_setup), execute(in_execute)
	{

	}
	explicit RenderGraphPass(
		CONST String& name,
		RenderGraph* in_rendergraph,
		MXRender::RHI::CommandList* in_cmd_list,
		CONST std::function<void(data_type&, RenderGraphPassBuilder&, MXRender::RHI::CommandList*)>& in_setup,
		CONST std::function<void(CONST data_type&, MXRender::RHI::CommandList*)>& in_execute) : RenderGraphPassBase(name), render_graph(in_rendergraph), cmd_list(in_cmd_list), setup(in_setup), execute(in_execute)
	{

	}
	RenderGraphPass() DEFAULT;
	VIRTUAL ~RenderGraphPass() DEFAULT;
	RenderGraphPass(RenderGraphPass&& temp) DEFAULT;
	RenderGraphPass& operator=(CONST RenderGraphPass& that) DELETE;
	RenderGraphPass& operator=(RenderGraphPass&& temp) DEFAULT;

	CONST data_type& GetData() CONST
	{
		return data;
	}
protected:
	void SetUp(RenderGraphPassBuilder& builder)       override
	{
		setup(data, builder,cmd_list);
	}
	void Execute() override
	{
		execute(data,cmd_list);
	}
private:

#pragma endregion

#pragma region MEMBER

public:

protected:
	RenderGraph* render_graph;
	MXRender::RHI::CommandList* cmd_list;
	data_type                                                         data;
	CONST std::function<void(data_type&, RenderGraphPassBuilder& , MXRender::RHI::CommandList*)>	  setup;
	CONST std::function<void(CONST data_type&, MXRender::RHI::CommandList*)>                       execute;
private:
#pragma endregion


MYRENDERER_END_CLASS


MYRENDERER_BEGIN_CLASS(RenderGraphPassBuilder)
#pragma region MATHOD

public:
	explicit RenderGraphPassBuilder(RenderGraph* in_rendergraph, RenderGraphPassBase* in_render_pass) ;
	RenderGraphPassBuilder() DEFAULT;
	VIRTUAL ~RenderGraphPassBuilder() DEFAULT;
	RenderGraphPassBuilder(RenderGraphPassBuilder&& temp) DEFAULT;
	RenderGraphPassBuilder& operator=(CONST RenderGraphPassBuilder& that) DEFAULT;
	RenderGraphPassBuilder& operator=(RenderGraphPassBuilder&& temp) DEFAULT;

	template<typename resource_type, typename description_type>
	resource_type* METHOD(Create)(CONST std::string& name, CONST description_type& description);
	template<typename resource_type>
	resource_type* METHOD(Read)(resource_type* resource);
	template<typename resource_type>
	resource_type* METHOD(Write)(resource_type* resource);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
protected:
	RenderGraph* graph;
	RenderGraphPassBase* pass;
private:

#pragma endregion
MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif