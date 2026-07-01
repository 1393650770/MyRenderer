// Self-contained unit tests for GraphValidator.
// Compile with: cl /EHsc /std:c++20 /I ../src/Runtime /I ../src/Editor test_graphvalidator.cpp
// Or include in the Editor project as a --test mode.

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include <cassert>
#include <iostream>

using namespace MXRender;
using namespace MXRender::Render;

static Int g_passed = 0;
static Int g_failed = 0;

#define TEST(name) void test_##name(); struct Register_##name { Register_##name() { test_##name(); } } reg_##name; void test_##name()
#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL: " << __FUNCTION__ << " line " << __LINE__ << std::endl; g_failed++; return; } } while(0)
#define PASS() g_passed++

TEST(empty_graph)
{
	RenderGraphDefinition def;
	auto result = GraphValidator::Validate(def);
	CHECK(result.is_valid);
	CHECK(result.errors.empty());
	PASS();
}

TEST(simple_valid)
{
	RenderGraphDefinition def;
	RDGPassDef p1; p1.name = "Pass1"; p1.write_resources.push_back("RT1"); def.passes.push_back(p1);
	RDGPassDef p2; p2.name = "Pass2"; p2.read_resources.push_back("RT1"); def.passes.push_back(p2);
	RDGResourceDef r1; r1.name = "RT1"; def.resources.push_back(r1);
	auto result = GraphValidator::Validate(def);
	CHECK(result.is_valid);
	PASS();
}

TEST(duplicate_names)
{
	RenderGraphDefinition def;
	RDGPassDef p1; p1.name = "SamePass"; def.passes.push_back(p1);
	RDGPassDef p2; p2.name = "SamePass"; def.passes.push_back(p2);
	auto result = GraphValidator::Validate(def);
	CHECK(!result.is_valid);
	PASS();
}

TEST(cycle_detection)
{
	RenderGraphDefinition def;
	RDGPassDef p1; p1.name = "A"; p1.write_resources.push_back("R"); p1.read_resources.push_back("R2"); def.passes.push_back(p1);
	RDGPassDef p2; p2.name = "B"; p2.write_resources.push_back("R2"); p2.read_resources.push_back("R"); def.passes.push_back(p2);
	RDGResourceDef r1; r1.name = "R"; def.resources.push_back(r1);
	RDGResourceDef r2; r2.name = "R2"; def.resources.push_back(r2);
	auto result = GraphValidator::Validate(def);
	CHECK(!result.is_valid);
	PASS();
}

TEST(unconnected_read)
{
	RenderGraphDefinition def;
	RDGPassDef p1; p1.name = "Pass1"; p1.read_resources.push_back("GhostRT"); def.passes.push_back(p1);
	auto result = GraphValidator::Validate(def);
	CHECK(!result.is_valid);
	PASS();
}

TEST(orphaned_output)
{
	RenderGraphDefinition def;
	RDGPassDef p1; p1.name = "Pass1"; p1.write_resources.push_back("UnreadRT"); def.passes.push_back(p1);
	RDGResourceDef r1; r1.name = "UnreadRT"; def.resources.push_back(r1);
	auto result = GraphValidator::Validate(def);
	CHECK(result.is_valid); // warnings only, not errors
	CHECK(!result.warnings.empty());
	PASS();
}

Int main()
{
	std::cout << "Running GraphValidator tests..." << std::endl;
	std::cout << "Passed: " << g_passed << ", Failed: " << g_failed << std::endl;
	return g_failed > 0 ? 1 : 0;
}
