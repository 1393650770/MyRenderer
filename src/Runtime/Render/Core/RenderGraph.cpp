#include "RenderGraph.h"
#include <xutility>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


void RenderGraph::Compile()
{
	// Reference counting.
	for (auto& pass : passes)
		pass->ref_count = pass->create_resources.size() + pass->write_resources.size();
	for (auto& resource : passes)
		resource->ref_count = resource->read_resources.size();

	// Culling via flood fill from unreferenced resources.
	std::stack<RenderGraphResourceBase*> unreferenced_resources;
	for (auto& resource : resources)
		if (resource->ref_count == 0 && resource->GetIsTransient())
			unreferenced_resources.push(resource.get());

	while (!unreferenced_resources.empty())
	{
		auto unreferenced_resource = unreferenced_resources.top();
		unreferenced_resources.pop();

		auto creator = const_cast<RenderGraphPassBase*>(unreferenced_resource->create_pass);
		if (creator->ref_count > 0)
			creator->ref_count--;
		if (creator->ref_count == 0 && creator->GetIsCullable())
		{
			for (auto iteratee : creator->read_resources)
			{
				auto read_resource = const_cast<RenderGraphResourceBase*>(iteratee);
				if (read_resource->ref_count> 0)
					read_resource->ref_count--;
				if (read_resource->ref_count== 0 && read_resource->GetIsTransient())
					unreferenced_resources.push(read_resource);
			}
		}

		for (auto c_writer : unreferenced_resource->write_passes)
		{
			auto writer = const_cast<RenderGraphPassBase*>(c_writer);
			if (writer->ref_count> 0)
				writer->ref_count--;
			if (writer->ref_count== 0 && writer->GetIsCullable())
			{
				for (auto iteratee : writer->read_resources)
				{
					auto read_resource = const_cast<RenderGraphResourceBase*>(iteratee);
					if (read_resource->ref_count> 0)
						read_resource->ref_count--;
					if (read_resource->ref_count == 0 && read_resource->GetIsTransient())
						unreferenced_resources.push(read_resource);
				}
			}
		}
	}


	// Timeline computation.
	steps.clear();
	for (auto& render_task : passes)
	{
		if (render_task->ref_count== 0 && render_task->GetIsCullable())
			continue;

		std::vector<RenderGraphResourceBase*> realized_resources, derealized_resources;

		for (auto resource : render_task->create_resources)
		{
			realized_resources.push_back(const_cast<RenderGraphResourceBase*>(resource));
			if (resource->read_passes.empty() && resource->write_passes.empty())
				derealized_resources.push_back(const_cast<RenderGraphResourceBase*>(resource));
		}

		auto reads_writes = render_task->read_resources;
		reads_writes.insert(reads_writes.end(), render_task->write_resources.begin(), render_task->write_resources.end());
		for (auto resource : reads_writes)
		{
			if (!resource->GetIsTransient())
				continue;

			auto        valid = false;
			std::size_t last_index;
			if (!resource->read_passes.empty())
			{
				auto last_reader = std::find_if(
					passes.begin(),
					passes.end(),
					[&resource](const std::unique_ptr<RenderGraphPassBase>& iteratee)
				{
					return iteratee.get() == resource->read_passes.back();
				});
				if (last_reader != passes.end())
				{
					valid = true;
					last_index = std::distance(passes.begin(), last_reader);
				}
			}
			if (!resource->write_passes.empty())
			{
				auto last_writer = std::find_if(
					passes.begin(),
					passes.end(),
					[&resource](const std::unique_ptr<RenderGraphPassBase>& iteratee)
				{
					return iteratee.get() == resource->write_passes.back();
				});
				if (last_writer != passes.end())
				{
					valid = true;
					last_index = max(last_index, std::size_t(std::distance(passes.begin(), last_writer)));
				}
			}

			if (valid && passes[last_index] == render_task)
				derealized_resources.push_back(const_cast<RenderGraphResourceBase*>(resource));
		}

		steps.push_back(RenderGraphStep{ render_task.get(), realized_resources, derealized_resources });
	}
}

void RenderGraph::Execute()
{
	for (auto& step : steps)
	{
		for (auto resource : step.realized_resources) 
			resource->Realize();
		step.pass->Execute();
		for (auto resource : step.derealized_resources) 
			resource->Derealize();
	}
}

void RenderGraph::Release()
{
	passes.clear();
	resources.clear();
}

bool RenderGraph::Searilize(CONST String& filename)
{
	return false;
}

bool RenderGraph::Desearilize(CONST String& filename)
{
	return false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
