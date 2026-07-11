#include "UI/RenderGraphEditor/Commands/NewGraphCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

NewGraphCmd::NewGraphCmd(RenderGraphPanel* in_panel)
	: panel(in_panel)
{
}

void NewGraphCmd::Execute()
{
	if (is_executed) return;
	is_executed = true;

	// Save snapshot for undo
	saved_nodes = panel->GetNodes();
	saved_links = panel->GetLinks();
	saved_selection = panel->GetSelectedNode();

	// Clear everything
	auto& nodes = panel->GetNodes();
	auto& links = panel->GetLinks();

	while (!links.empty())
	{
		auto* link = links.back();
		link->Release();
		delete link;
		links.pop_back();
	}

	while (!nodes.empty())
	{
		auto* node = nodes.back();
		node->Release();
		delete node;
		nodes.pop_back();
	}

	panel->SetSelectedNode(nullptr);
}

void NewGraphCmd::Undo()
{
	if (!is_executed) return;
	is_executed = false;

	// Restore snapshot
	auto& nodes = panel->GetNodes();
	auto& links = panel->GetLinks();

	// Clear current state
	while (!links.empty()) { delete links.back(); links.pop_back(); }
	while (!nodes.empty()) { delete nodes.back(); nodes.pop_back(); }

	nodes = saved_nodes;
	links = saved_links;
	panel->SetSelectedNode(saved_selection);

	saved_nodes.clear();
	saved_links.clear();
	saved_selection = nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
