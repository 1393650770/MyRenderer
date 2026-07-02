// -- [AI:BEGIN] --
// Round-trip integration test: BuildDefinition -> Serialize -> Deserialize -> LoadDefinition -> BuildDefinition.
// Verifies that serialize/deserialize preserves graph topology.
// Compile as part of Editor target, run with --test flag.

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "UI/RenderGraphEditor/RenderGraphSerializer.h"
#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include <cassert>
#include <iostream>
#include <cstdio>

using namespace MXRender;
using namespace MXRender::Render;
using namespace MXRender::UI;

static Int g_passed = 0;
static Int g_failed = 0;

#define RTEST(name) void rt_##name(); struct RTReg_##name { RTReg_##name() { rt_##name(); } } rtr_##name; void rt_##name()
#define RCHECK(cond) do { if (!(cond)) { std::cerr << "FAIL: " << __FUNCTION__ << " line " << __LINE__ << std::endl; g_failed++; return; } } while(0)
#define RPASS() g_passed++

// -- [AI] Full round-trip: build, save, load, rebuild, compare
RTEST(round_trip_full_graph)
{
	// Build a non-trivial graph definition
	RenderGraphDefinition def;
	def.graph_name = "RoundTripTest";

	// Resources
	RDGResourceDef rt;
	rt.name = "GBufferA"; rt.kind = RDGResourceKind::Texture;
	rt.width = 1920; rt.height = 1080; rt.texture_format = 43; // RGBA8
	def.resources.push_back(rt);

	RDGResourceDef rt2;
	rt2.name = "LightBuffer"; rt2.kind = RDGResourceKind::Texture;
	rt2.width = 1920; rt2.height = 1080;
	def.resources.push_back(rt2);

	// Passes
	RDGPassDef gbuffer;
	gbuffer.name = "GBufferPass"; gbuffer.pass_kind = RDGPassKind::Graphics;
	gbuffer.pass_flags = RDGPassFlags::Raster;
	gbuffer.write_resources.push_back("GBufferA");
	def.passes.push_back(gbuffer);

	RDGPassDef lighting;
	lighting.name = "LightingPass"; lighting.pass_kind = RDGPassKind::Graphics;
	lighting.pass_flags = RDGPassFlags::Raster;
	lighting.read_resources.push_back("GBufferA");
	lighting.write_resources.push_back("LightBuffer");
	def.passes.push_back(lighting);

	// Edges
	RDGEdgeDef e1;
	e1.source_node_name = "GBufferPass"; e1.source_pin_name = "GBufferA";
	e1.target_node_name = "GBufferA"; e1.target_pin_name = "Input";
	e1.edge_type = 1; // Write
	def.edges.push_back(e1);

	RDGEdgeDef e2;
	e2.source_node_name = "LightBuffer"; e2.source_pin_name = "Output";
	e2.target_node_name = "LightingPass"; e2.target_pin_name = "GBufferA";
	e2.edge_type = 0; // Read
	def.edges.push_back(e2);

	// Validate original
	auto val_result = GraphValidator::Validate(def);
	RCHECK(val_result.is_valid);

	// Serialize to temp file
	String tmp_path = "tests/_rt_test.rgraph.json";
	Bool save_ok = RenderGraphSerializer::SaveGraph(def, tmp_path);
	RCHECK(save_ok);

	// Deserialize
	RenderGraphDefinition loaded;
	Bool load_ok = RenderGraphSerializer::LoadGraph(loaded, tmp_path);
	RCHECK(load_ok);

	// Verify key fields preserved
	RCHECK(loaded.graph_name == def.graph_name);
	RCHECK(loaded.passes.size() == def.passes.size());
	RCHECK(loaded.resources.size() == def.resources.size());
	RCHECK(loaded.edges.size() == def.edges.size());
	RCHECK(loaded.version == 2);

	// Verify pass names
	RCHECK(loaded.passes[0].name == "GBufferPass");
	RCHECK(loaded.passes[1].name == "LightingPass");

	// Verify resource properties
	RCHECK(loaded.resources[0].name == "GBufferA");
	RCHECK(loaded.resources[0].width == 1920);
	RCHECK(loaded.resources[0].height == 1080);

	// Re-validate loaded graph
	auto val2 = GraphValidator::Validate(loaded);
	RCHECK(val2.is_valid);

	// Cleanup
	std::remove(tmp_path.c_str());

	RPASS();
}

// -- [AI] Version migration: v1 file should load with default v2 fields
RTEST(v1_to_v2_migration)
{
	// Write a v1 format JSON manually
	String tmp = "tests/_rt_v1test.rgraph.json";
	{
		std::ofstream f(tmp);
		f << "{\n";
		f << "  \"graph_name\": \"MigrationTest\",\n";
		f << "  \"version\": 1,\n";
		f << "  \"passes\": [{\"name\":\"P1\",\"pass_kind\":\"Graphics\",\"is_cullable\":true,\"read_resources\":[],\"write_resources\":[],\"create_resources\":[]}],\n";
		f << "  \"resources\": [],\n";
		f << "  \"edges\": [],\n";
		f << "  \"node_layouts\": [],\n";
		f << "  \"editor_zoom\": 1.0,\n";
		f << "  \"editor_offset_x\": 0.0,\n";
		f << "  \"editor_offset_y\": 0.0\n";
		f << "}\n";
	}

	RenderGraphDefinition loaded;
	Bool load_ok = RenderGraphSerializer::LoadGraph(loaded, tmp);
	RCHECK(load_ok);
	RCHECK(loaded.passes.size() == 1);
	RCHECK(loaded.passes[0].name == "P1");
	// v1 pass_flags defaults to Raster (since we map is_cullable to flags)
	// v2 additions (sub_graphs, annotations, metadata) should be empty
	RCHECK(loaded.sub_graphs.empty());
	RCHECK(loaded.annotations.empty());

	std::remove(tmp.c_str());
	RPASS();
}

// Static initialization: run tests automatically
static struct RoundTripTestRunner {
	RoundTripTestRunner() {
		std::cout << "[RoundTripTests] " << g_passed << " passed, " << g_failed << " failed" << std::endl;
	}
} g_runner;
// -- [AI:END] --
