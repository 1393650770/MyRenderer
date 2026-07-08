#include "Render/Core/RenderGraphValidator.h"
#include <queue>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

void ValidationResult::Merge(CONST ValidationResult& other)
{
	if (!other.is_valid) is_valid = false;
	errors.insert(errors.end(), other.errors.begin(), other.errors.end());
	warnings.insert(warnings.end(), other.warnings.begin(), other.warnings.end());
}

Bool RenderGraphValidator::DetectCycles(CONST RenderGraphDefinition& def, ValidationResult& out_result)
{
	Map<String, Vector<String>> adjacency;
	Map<String, UInt32> in_degree;
	Set<String> all_passes;

	for (auto& pd : def.passes)
	{
		all_passes.insert(pd.name);
		in_degree[pd.name] = 0;
		adjacency[pd.name] = {};
		for (auto& rname : pd.write_resources)
		{
			for (auto& rp : def.passes)
			{
				if (rp.name == pd.name) continue;
				for (auto& rr : rp.read_resources)
					if (rr == rname) adjacency[pd.name].push_back(rp.name);
			}
		}
		for (auto& rname : pd.create_resources)
		{
			for (auto& rp : def.passes)
			{
				if (rp.name == pd.name) continue;
				for (auto& rr : rp.read_resources)
					if (rr == rname) adjacency[pd.name].push_back(rp.name);
			}
		}
	}

	for (auto& kv : adjacency)
		for (auto& succ : kv.second)
			in_degree[succ]++;

	Queue<String> q;
	for (auto& pd : def.passes)
		if (in_degree[pd.name] == 0) q.push(pd.name);

	UInt32 visited = 0;
	while (!q.empty()) {
		String current = q.front(); q.pop(); visited++;
		for (auto& succ : adjacency[current]) {
			in_degree[succ]--;
			if (in_degree[succ] == 0) q.push(succ);
		}
	}

	if (visited < all_passes.size()) {
		out_result.is_valid = false;
		ValidationError err;
		err.message = "Graph contains a cycle: " + std::to_string(all_passes.size() - visited) + " unreachable passes";
		out_result.errors.push_back(err);
		return false;
	}
	return true;
}

void RenderGraphValidator::CheckPinConnectivity(CONST RenderGraphDefinition& def, ValidationResult& out_result)
{
	for (auto& pd : def.passes)
	{
		for (auto& rname : pd.read_resources)
		{
			Bool has_producer = false;
			for (auto& rp : def.passes)
			{
				if (rp.name == pd.name) continue;
				for (auto& cr : rp.create_resources) if (cr == rname) { has_producer = true; break; }
				for (auto& wr : rp.write_resources) if (wr == rname) { has_producer = true; break; }
				if (has_producer) break;
			}
			if (!has_producer)
			{
				Bool is_retained = false;
				for (auto& rd : def.resources) if (rd.name == rname && !rd.is_transient) { is_retained = true; break; }
				if (!is_retained)
				{
					ValidationError err;
					err.message = "Pass " + pd.name + " reads resource " + rname + " which has no producer";
					err.node_name = pd.name;
					out_result.errors.push_back(err);
					out_result.is_valid = false;
				}
			}
		}
	}
}

void RenderGraphValidator::CheckBipartiteRule(CONST RenderGraphDefinition& def, ValidationResult& out_result)
{
	Set<String> pass_names, res_names;
	for (auto& pd : def.passes) pass_names.insert(pd.name);
	for (auto& rd : def.resources) res_names.insert(rd.name);

	for (auto& ed : def.edges)
	{
		Bool s_pass = pass_names.count(ed.source_node_name) > 0;
		Bool t_pass = pass_names.count(ed.target_node_name) > 0;
		Bool s_res = res_names.count(ed.source_node_name) > 0;
		Bool t_res = res_names.count(ed.target_node_name) > 0;

		if ((s_pass && t_pass) || (s_res && t_res))
		{
			ValidationError err;
			err.message = "Direct " + String(s_pass ? "Pass-Pass" : "Resource-Resource") + " connection: " + ed.source_node_name + " -> " + ed.target_node_name;
			out_result.errors.push_back(err);
			out_result.is_valid = false;
		}
	}
}

void RenderGraphValidator::CheckNamingConflicts(CONST RenderGraphDefinition& def, ValidationResult& out_result)
{
	Set<String> names;
	for (auto& pd : def.passes) {
		if (names.count(pd.name)) {
			ValidationError err; err.message = "Duplicate pass: " + pd.name;
			err.node_name = pd.name; out_result.errors.push_back(err); out_result.is_valid = false;
		}
		names.insert(pd.name);
	}
	for (auto& rd : def.resources) {
		if (names.count(rd.name)) {
			ValidationError err; err.message = "Duplicate resource: " + rd.name;
			err.node_name = rd.name; out_result.errors.push_back(err); out_result.is_valid = false;
		}
		names.insert(rd.name);
	}
}

void RenderGraphValidator::CheckOrphanedOutputs(CONST RenderGraphDefinition& def, ValidationResult& out_result)
{
	for (auto& pd : def.passes)
	{
		for (auto& rname : pd.write_resources)
		{
			Bool has_consumer = false;
			for (auto& rp : def.passes)
			{
				if (rp.name == pd.name) continue;
				for (auto& rr : rp.read_resources) if (rr == rname) { has_consumer = true; break; }
				if (has_consumer) break;
			}
			if (!has_consumer) {
				ValidationWarning warn;
				warn.message = "Pass " + pd.name + " writes resource " + rname + " with no consumer";
				warn.node_name = pd.name; out_result.warnings.push_back(warn);
			}
		}
	}
}

ValidationResult RenderGraphValidator::Validate(CONST RenderGraphDefinition& def)
{
	ValidationResult result;
	DetectCycles(def, result);
	if (!result.is_valid) return result;
	CheckPinConnectivity(def, result);
	CheckBipartiteRule(def, result);
	CheckNamingConflicts(def, result);
	CheckOrphanedOutputs(def, result);
	return result;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
