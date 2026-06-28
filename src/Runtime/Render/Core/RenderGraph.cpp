#include "RenderGraph.h"
#include <xutility>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


void RenderGraph::CompileAliasingPlan()
{
	aliasing_offsets.clear();
	aliasing_block_size = 0;

	// Collect transient buffers with their lifetime intervals.
	struct BufferAliasInfo
	{
		RenderGraphResourceBase* resource;
		UInt32 start_step;
		UInt32 end_step;
		UInt64 size;
		UInt32 alignment;
	};
	Vector<BufferAliasInfo> infos;

	for (UInt32 s = 0; s < steps.size(); ++s)
	{
		for (auto* res : steps[s].realized_resources)
		{
			if (!res->GetIsTransient() || !res->IsBufferResource()) continue;
			if (aliasing_offsets.count(res)) continue; // Already processed

			// Find derealization step
			UInt32 end = s;
			for (UInt32 e = s; e < steps.size(); ++e)
			{
				for (auto* dres : steps[e].derealized_resources)
				{
					if (dres == res) { end = e; break; }
				}
			}

			// Get buffer size from description
			UInt64 buf_size = 256; // Default
			auto* buf_res = static_cast<RenderGraphResource<RHI::BufferDesc, RHI::Buffer>*>(res);
			buf_size = buf_res->GetDescription().size;
			UInt32 alignment = 256;

			infos.push_back({ res, s, end, buf_size, alignment });
		}
	}

	if (infos.empty()) return;

	// Sort by size descending (simple bubble — avoids <functional> template issues)
	for (UInt32 i = 0; i < infos.size(); ++i)
		for (UInt32 j = i + 1; j < infos.size(); ++j)
			if (infos[j].size > infos[i].size) { auto tmp = infos[i]; infos[i] = infos[j]; infos[j] = tmp; }

	// Simple greedy placement: place each buffer at the first offset that doesn't overlap
	// with any previously placed buffer with overlapping lifetime
	struct PlacedBuffer { UInt64 offset; UInt64 size; UInt32 start; UInt32 end; };
	Vector<PlacedBuffer> placed;

	for (auto& info : infos)
	{
		UInt64 best_offset = 0;
		bool found = false;

		// Try placements at each possible offset
		for (UInt32 attempt = 0; attempt < 100 && !found; ++attempt)
		{
			UInt64 candidate = best_offset;
			bool overlaps = false;
			for (auto& p : placed)
			{
				// Check lifetime overlap
				if (info.start_step <= p.end && p.start <= info.end_step)
				{
					// Check offset overlap
					if (candidate < p.offset + p.size && candidate + info.size > p.offset)
					{
						candidate = p.offset + p.size; // Move past this buffer
						overlaps = true;
					}
				}
			}
			if (!overlaps) found = true;
			best_offset = candidate;
		}

		UInt64 aligned = (best_offset + info.alignment - 1) & ~(UInt64)(info.alignment - 1);
		aliasing_offsets[info.resource] = aligned;
		placed.push_back({ aligned, info.size, info.start_step, info.end_step });
		aliasing_block_size = (std::max)(aliasing_block_size, aligned + info.size);
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

void RenderGraph::Compile()
{
	// Reference counting.
	for (auto& pass : passes)
		pass->ref_count = pass->create_resources.size() + pass->write_resources.size();
	for (auto& resource : resources)
		resource->ref_count = resource->read_passes.size();

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
				if (read_resource->ref_count > 0)
					read_resource->ref_count--;
				if (read_resource->ref_count == 0 && read_resource->GetIsTransient())
					unreferenced_resources.push(read_resource);
			}
		}

		for (auto c_writer : unreferenced_resource->write_passes)
		{
			auto writer = const_cast<RenderGraphPassBase*>(c_writer);
			if (writer->ref_count > 0)
				writer->ref_count--;
			if (writer->ref_count == 0 && writer->GetIsCullable())
			{
				for (auto iteratee : writer->read_resources)
				{
					auto read_resource = const_cast<RenderGraphResourceBase*>(iteratee);
					if (read_resource->ref_count > 0)
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
		if (render_task->ref_count == 0 && render_task->GetIsCullable())
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

			auto valid = false;
			std::size_t last_index;
			if (!resource->read_passes.empty())
			{
				auto last_reader = std::find_if(passes.begin(), passes.end(),
					[&resource](const std::unique_ptr<RenderGraphPassBase>& iteratee)
					{ return iteratee.get() == resource->read_passes.back(); });
				if (last_reader != passes.end()) { valid = true; last_index = std::distance(passes.begin(), last_reader); }
			}
			if (!resource->write_passes.empty())
			{
				auto last_writer = std::find_if(passes.begin(), passes.end(),
					[&resource](const std::unique_ptr<RenderGraphPassBase>& iteratee)
					{ return iteratee.get() == resource->write_passes.back(); });
				if (last_writer != passes.end()) { valid = true; last_index = (std::max)(last_index, std::size_t(std::distance(passes.begin(), last_writer))); }
			}

			if (valid && passes[last_index] == render_task)
				derealized_resources.push_back(const_cast<RenderGraphResourceBase*>(resource));
		}

		steps.push_back(RenderGraphStep{ render_task.get(), realized_resources, derealized_resources });
	}

	// Compute buffer aliasing plan after timeline
	CompileAliasingPlan();
}

void RenderGraph::Release()
{
	passes.clear();
	resources.clear();
	aliasing_offsets.clear();
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
