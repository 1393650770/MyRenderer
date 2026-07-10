#include "RenderGraph.h"
#include <xutility>
#include <fstream>
#include <functional>
#include <queue>
#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)


void RenderGraph::CompileAliasingPlan()
{
	aliasing_offsets.clear();
	aliasing_block_size = 0;

	struct AliasInfo
	{
		RenderGraphResourceBase* resource;
		UInt32 start_step;
		UInt32 end_step;
		UInt64 size;
		UInt32 alignment;
	};
	Vector<AliasInfo> infos;

	for (UInt32 s = 0; s < steps.size(); ++s)
	{
		for (auto* res : steps[s].realized_resources)
		{
			// --   Skip non-transient and already-processed resources
			if (!res->GetIsTransient()) continue;
			if (aliasing_offsets.count(res)) continue;
			if (!res->IsBufferResource() && !res->IsTextureResource()) continue;

			// Find derealization step
			UInt32 end = s;
			for (UInt32 e = s; e < steps.size(); ++e)
				for (auto* dres : steps[e].derealized_resources)
					if (dres == res) { end = e; break; }

			// --   Compute resource size for both buffer and texture
			UInt64 res_size = 256;
			UInt32 alignment = 256;
			if (res->IsBufferResource()) {
				auto* br = static_cast<RenderGraphResource<RHI::BufferDesc, RHI::Buffer>*>(res);
				res_size = br->GetDescription().size;
				alignment = 256;
			} else {
				auto* tr = static_cast<RenderGraphResource<RHI::TextureDesc, RHI::Texture>*>(res);
				RHI::TextureDesc d = tr->GetDescription();
				res_size = (UInt64)d.width * d.height * 4;
				alignment = 64;
			}

			infos.push_back({ res, s, end, res_size, alignment });
		}
	}

	if (infos.empty()) return;

	for (UInt32 i = 0; i < infos.size(); ++i)
		for (UInt32 j = i + 1; j < infos.size(); ++j)
			if (infos[j].size > infos[i].size) { auto tmp = infos[i]; infos[i] = infos[j]; infos[j] = tmp; }

	struct Placed { UInt64 offset; UInt64 size; UInt32 start; UInt32 end; };
	Vector<Placed> placed;

	for (auto& info : infos)
	{
		UInt64 best_offset = 0;
		bool found = false;

		for (UInt32 attempt = 0; attempt < 100 && !found; ++attempt)
		{
			UInt64 candidate = best_offset;
			bool overlaps = false;
			for (auto& p : placed)
			{
				if (info.start_step <= p.end && p.start <= info.end_step)
					if (candidate < p.offset + p.size && candidate + info.size > p.offset)
					{
						candidate = p.offset + p.size;
						overlaps = true;
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

// --   Compile with config (safe mode support)
void RenderGraph::Compile(CONST RenderGraphCompileConfig& config)
{
	// In safe mode, skip barriers and use conservative path
	if (config.fallback_to_conservative) {
		Compile(); // Basic compile, no barriers, no aliasing
		return;
	}
	Compile();
}

void RenderGraph::Execute()
{
	// --   Get command list for barrier submission
	auto* cmd_list = RHIGetImmediateCommandList();
	UInt32 ts_idx = 0;

	for (auto& step : steps)
	{
		for (auto resource : step.realized_resources)
		{
			resource->Realize();
			// --   Set debug object name for RenderDoc
			String dbg_name = step.pass->GetName() + ":" + resource->GetName();
			if (auto* tex = resource->GetAsTexture())
				SetDebugNameForRHIResource(tex, dbg_name);
			else if (auto* buf = resource->GetAsBuffer())
				SetDebugNameForRHIResource(buf, dbg_name);
		}

		// --   Submit prologue barriers
		for (auto& bd : step.prologue_barriers)
		{
			if (!bd.resource || !cmd_list) continue;
			if (bd.src_state == bd.dst_state) continue;
			if (auto* tex = bd.resource->GetAsTexture())
				cmd_list->TransitionTextureState(tex, bd.dst_state);
			else if (bd.resource->GetAsBuffer())
				cmd_list->ResourceBarrier(bd.src_state, bd.dst_state);
			else
				cmd_list->ResourceBarrier(bd.src_state, bd.dst_state);
		}

		// --   GPU timing: timestamp before pass (if cmd_list available)
		if (cmd_list) cmd_list->WriteTimestamp(ts_idx++);

		step.pass->Execute(cmd_list);

		// --   GPU timing: timestamp after pass
		if (cmd_list) cmd_list->WriteTimestamp(ts_idx++);

		// --   Submit epilogue barriers
		for (auto& bd : step.epilogue_barriers)
		{
			if (!bd.resource || !cmd_list) continue;
			if (bd.src_state == bd.dst_state) continue;
			if (auto* tex = bd.resource->GetAsTexture())
				cmd_list->TransitionTextureState(tex, bd.dst_state);
			else if (bd.resource->GetAsBuffer())
				cmd_list->ResourceBarrier(bd.src_state, bd.dst_state);
			else
				cmd_list->ResourceBarrier(bd.src_state, bd.dst_state);
		}

		for (auto resource : step.derealized_resources)
			resource->Derealize();
	}

	// Process deferred destruction each frame
	ProcessDeferredDestruction();
}

void RenderGraph::Compile()
{
	// Reference counting.
	for (auto& pass : passes)
		pass->ref_count = pass->create_resources.size() + pass->write_resources.size();
	for (auto& resource : resources)
		resource->ref_count = resource->read_passes.size();

	// --   Compile cache — skip if topology unchanged
	static UInt64 s_cached_hash = ~0ULL;
	static Vector<RenderGraphStep> s_cached_steps;
	UInt64 th = 0;
	for (auto& p : passes) th ^= std::hash<String>{}(p->GetName()) ^ (UInt64)(uintptr_t)p.get();
	for (auto& r : resources) th ^= std::hash<String>{}(r->GetName());
	if (th == s_cached_hash && !s_cached_steps.empty()) {
		steps = s_cached_steps; return;
	}
	s_cached_hash = th;

	// --   Topological sort + cycle detection (Kahn BFS)
	Map<RenderGraphPassBase*, Vector<RenderGraphPassBase*>> adj;
	Map<RenderGraphPassBase*, UInt32> in_deg;
	for (auto& pass : passes) { adj[pass.get()] = {}; in_deg[pass.get()] = 0; }

	for (auto& resource : resources)
	{
		if (!resource->create_pass) continue;
		auto* producer = const_cast<RenderGraphPassBase*>(resource->create_pass);
		for (auto* reader : resource->read_passes) {
			auto* r = const_cast<RenderGraphPassBase*>(reader);
			if (r != producer) adj[producer].push_back(r);
		}
		for (auto* writer : resource->write_passes) {
			auto* w = const_cast<RenderGraphPassBase*>(writer);
			if (w != producer) adj[producer].push_back(w);
		}
	}

	for (auto& kv : adj)
		for (auto* succ : kv.second) in_deg[succ]++;

	std::queue<RenderGraphPassBase*> q;
	for (auto& kv : in_deg)
		if (kv.second == 0) q.push(kv.first);

	UInt32 visited = 0;
	while (!q.empty()) {
		auto* cur = q.front(); q.pop(); visited++;
		for (auto* succ : adj[cur]) {
			in_deg[succ]--;
			if (in_deg[succ] == 0) q.push(succ);
		}
	}

	if (visited < passes.size()) {
		std::cerr << "[RenderGraph] Cycle: " << (passes.size() - visited) << " unreachable passes" << std::endl;
		return;
	}
	// --   --

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
		if (!creator) continue;
		if (creator->ref_count > 0) creator->ref_count--;
		if (creator->ref_count == 0 && creator->GetIsCullable())
		{
			for (auto iteratee : creator->read_resources)
			{
				auto read_resource = const_cast<RenderGraphResourceBase*>(iteratee);
				if (read_resource->ref_count > 0) read_resource->ref_count--;
				if (read_resource->ref_count == 0 && read_resource->GetIsTransient())
					unreferenced_resources.push(read_resource);
			}
		}

		for (auto c_writer : unreferenced_resource->write_passes)
		{
			auto writer = const_cast<RenderGraphPassBase*>(c_writer);
			if (writer->ref_count > 0) writer->ref_count--;
			if (writer->ref_count == 0 && writer->GetIsCullable())
			{
				for (auto iteratee : writer->read_resources)
				{
					auto read_resource = const_cast<RenderGraphResourceBase*>(iteratee);
					if (read_resource->ref_count > 0) read_resource->ref_count--;
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
		if (render_task->ref_count == 0 && render_task->GetIsCullable()) continue;

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
			if (!resource->GetIsTransient()) continue;

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

	// --   Barrier generation with stamp tracking
	// Auto-derive access_sequence for resources whose passes didn't explicitly
	// declare required states via Read(res, state) / Write(res, state).
	// This walks the compiled step order so that access_sequence entries are
	// ordered by pass execution, which the barrier generation below relies on.
	for (auto& step : steps)
	{
		for (auto* r : step.pass->create_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			Bool has_explicit = false;
			for (auto& acc : res->access_sequence)
				if (acc.pass == step.pass) { has_explicit = true; break; }
			if (has_explicit) continue;
			ENUM_RESOURCE_STATE init_state = ENUM_RESOURCE_STATE::ShaderResource;
			for (auto* w : res->write_passes)
				if (w == step.pass) { init_state = ENUM_RESOURCE_STATE::RenderTarget; break; }
			res->access_sequence.push_back({ step.pass, init_state, false, 0 });
			res->tracked_state = ENUM_RESOURCE_STATE::Undefined;
		}
		for (auto* r : step.pass->read_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			if (!res->GetIsTransient()) continue;
			Bool has_explicit = false;
			for (auto& acc : res->access_sequence)
				if (acc.pass == step.pass) { has_explicit = true; break; }
			if (has_explicit) continue;
			res->access_sequence.push_back({ step.pass, ENUM_RESOURCE_STATE::ShaderResource, false, 0 });
		}
		for (auto* r : step.pass->write_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			if (!res->GetIsTransient()) continue;
			Bool has_explicit = false;
			for (auto& acc : res->access_sequence)
				if (acc.pass == step.pass) { has_explicit = true; break; }
			if (has_explicit) continue;
			res->access_sequence.push_back({ step.pass, ENUM_RESOURCE_STATE::RenderTarget, true, 0 });
		}
	}

	UInt64 global_stamp = 1;
	for (auto& step : steps)
	{
		for (auto* r : step.pass->read_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			if (!res->GetIsTransient()) continue;
			for (auto& acc : res->access_sequence)
				if (acc.pass == step.pass) { acc.modification_stamp = global_stamp; break; }
			if (!res->access_sequence.empty() && res->tracked_state != res->access_sequence.back().required_state) {
				RHIBarrierDesc bd; bd.resource = res;
				bd.src_state = res->tracked_state;
				bd.dst_state = res->access_sequence.back().required_state;
				bd.is_prologue = true;
				step.prologue_barriers.push_back(bd);
				res->tracked_state = res->access_sequence.back().required_state;
			}
		}
		for (auto* r : step.pass->write_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			if (!res->GetIsTransient()) continue;
			global_stamp++;
			for (auto& acc : res->access_sequence)
				if (acc.pass == step.pass) { acc.modification_stamp = global_stamp; acc.is_write = true; break; }
			if (!res->access_sequence.empty() && res->tracked_state != res->access_sequence.back().required_state) {
				RHIBarrierDesc bd; bd.resource = res;
				bd.src_state = res->tracked_state;
				bd.dst_state = res->access_sequence.back().required_state;
				bd.is_prologue = true;
				step.prologue_barriers.push_back(bd);
				res->tracked_state = res->access_sequence.back().required_state;
			}
		}
		for (auto* r : step.pass->create_resources) {
			auto* res = const_cast<RenderGraphResourceBase*>(r);
			global_stamp++; res->tracked_state = ENUM_RESOURCE_STATE::Undefined;
		}
	}
	// --   --

	// Compute aliasing plan after timeline + barriers
	CompileAliasingPlan();

	// Cache compiled steps
	s_cached_steps = steps;
}

void RenderGraph::ScheduleAsyncCompute()
{
	// --   Stub: count async-capable passes (those with "Compute" in name)
	UInt32 count = 0;
	for (auto& step : steps)
		if (step.pass->GetName().find("Compute") != String::npos) count++;
	if (count > 0)
		std::cout << "[RenderGraph] " << count << " async-compute candidates (stub)" << std::endl;
}

void RenderGraph::ExtractResource(RenderGraphResourceBase* resource)
{
	// --   Remove from derealization so it survives graph Release
	if (resource)
		for (auto& step : steps) {
			auto& dr = step.derealized_resources;
			dr.erase(std::remove(dr.begin(), dr.end(), resource), dr.end());
		}
}

void RenderGraph::Release()
{
	passes.clear();
	resources.clear();
	aliasing_offsets.clear();
}

void RenderGraph::DumpGraphViz(CONST String& path)
{
	std::ofstream f(path);
	if (!f) return;
	f << "digraph RenderGraph {" << std::endl;
	f << "  rankdir=LR;" << std::endl;
	for (auto& pass : passes)
		f << "  \"" << pass->GetName() << "\" [shape=box,style=filled,fillcolor=orange];" << std::endl;
	for (auto& res : resources) {
		String color = res->GetIsTransient() ? "lightblue" : "white";
		f << "  \"" << res->GetName() << "\" [shape=ellipse,style=filled,fillcolor=" << color << "];" << std::endl;
	}
	for (auto& step : steps) {
		for (auto* r : step.pass->read_resources)
			f << "  \"" << r->GetName() << "\" -> \"" << step.pass->GetName() << "\" [color=green];" << std::endl;
		for (auto* r : step.pass->write_resources)
			f << "  \"" << step.pass->GetName() << "\" -> \"" << r->GetName() << "\" [color=red];" << std::endl;
	}
	f << "}" << std::endl;
}

void RenderGraph::DebugDumpBarriers()
{
	for (auto& step : steps) {
		std::cout << "[" << step.pass->GetName() << "] prologue=" << step.prologue_barriers.size()
			<< " epilogue=" << step.epilogue_barriers.size() << std::endl;
		for (auto& bd : step.prologue_barriers)
			std::cout << "  PRO: " << (bd.resource ? bd.resource->GetName() : "?")
				<< " [" << (Int)bd.src_state << "->" << (Int)bd.dst_state << "]" << std::endl;
	}
}

bool RenderGraph::Searilize(CONST String& filename)
{
	// --   Delegate to editor-side JSON serializer via Definition
	return false;
}

bool RenderGraph::Desearilize(CONST String& filename)
{
	return false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
