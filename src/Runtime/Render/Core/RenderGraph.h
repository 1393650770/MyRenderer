#pragma once
#ifndef _RENDERGRAPH_
#define _RENDERGRAPH_
#include <memory>
#include "RenderGraphPass.h"
#include "RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


class RenderGraphPassBase;
class RenderGraphResourceBase;
MYRENDERER_BEGIN_CLASS(RenderGraph)

#pragma region MATHOD

public:
	RenderGraph() MYDEFAULT;
	VIRTUAL~RenderGraph() MYDEFAULT;
	RenderGraph(RenderGraph&& temp) MYDEFAULT;
	RenderGraph& operator=(CONST RenderGraph& that) MYDELETE;
	RenderGraph& operator=(RenderGraph&& temp) MYDEFAULT;

	template<typename data_type, typename... argument_types>
	RenderGraphPass<data_type>* AddRenderPass(argument_types&&... arguments)
	{
		passes.emplace_back(std::make_unique<RenderGraphPass<data_type>>(arguments...));
		auto render_task = passes.back().get();

		RenderGraphPassBuilder builder(this, render_task);
		render_task->SetUp(builder);

		return static_cast<RenderGraphPass<data_type>*>(render_task);
	}

	template<typename description_type, typename actual_type>
	RenderGraphResource<description_type, actual_type>* METHOD(AddRetainedResource)(const std::string& name, const description_type& description, actual_type* actual = nullptr)
	{
		resources.emplace_back(std::make_unique<RenderGraphResource<description_type, actual_type>>(name, description, actual));
		return static_cast<RenderGraphResource<description_type, actual_type>*>(resources.back().get());
	}

	template<typename description_type, typename actual_type>
	RenderGraphResource<description_type, actual_type>* METHOD( GetRetainedResource)(const std::string& name)
	{
		for (auto& resource : resources)
		{
			if (resource->GetName() == name)
			{
				return static_cast<RenderGraphResource<description_type, actual_type>*>(resource.get());
			}
		}
		return nullptr;
	}

	//A directed acyclic graph is constructed to find the resource reference relationship
	void METHOD(Compile)();

	void METHOD(Execute)();

	void METHOD(Release)();
protected:

private:

#pragma endregion

#pragma region MEMBER

public:

protected:
	friend class RenderGraphTaskBuilder;

	MYRENDERER_BEGIN_STRUCT(RenderGraphStep)
	public:
		RenderGraphPassBase* pass;
		Vector<RenderGraphResourceBase*> realized_resources;
		Vector<RenderGraphResourceBase*> derealized_resources;
	MYRENDERER_END_STRUCT


	Vector<std::unique_ptr<RenderGraphPassBase>> passes;
	Vector<std::unique_ptr<RenderGraphResourceBase>> resources;
	Vector<RenderGraphStep> steps;
private:

#pragma endregion

MYRENDERER_END_CLASS


template<typename resource_type>
resource_type* MXRender::Render::RenderGraphPassBuilder::Write(resource_type* resource)
{
	resource->write_passes.push_back(pass);
	pass->write_resources.push_back(resource);
	return resource;
}

template<typename resource_type>
resource_type* MXRender::Render::RenderGraphPassBuilder::Read(resource_type* resource)
{
	resource->read_passes.push_back(pass);
	pass->read_resources.push_back(resource);
	return resource;
}

template<typename resource_type, typename description_type>
resource_type* MXRender::Render::RenderGraphPassBuilder::Create(const std::string& name, const description_type& description)
{
	static_assert(std::is_same<typename resource_type::description_type, description_type>::value, "Description does not match the resource.");
	graph->resources.emplace_back(std::make_unique<resource_type>(name, pass, description));
	CONST auto resource = graph->resources.back().get();
	pass->create_resources.push_back(resource);
	return static_cast<resource_type*>(resource);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif

