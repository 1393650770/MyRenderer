#include "UI/RenderGraphEditor/Commands/OpenGraphCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "Render/Core/RenderGraphSerializer.h"
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

OpenGraphCmd::OpenGraphCmd(RenderGraphPanel* in_panel, const String& in_path)
	: panel(in_panel), file_path(in_path)
{
}

void OpenGraphCmd::Execute()
{
	if (is_executed) return;
	is_executed = true;

	// Save snapshot
	saved_nodes = panel->GetNodes();
	saved_links = panel->GetLinks();
	saved_selection = panel->GetSelectedNode();

	// Load new definition
	Render::RenderGraphDefinition def;
	if (Render::RenderGraphSerializer::LoadGraph(def, file_path))
	{
		panel->LoadDefinition(def);
		loaded_nodes = panel->GetNodes();
		loaded_links = panel->GetLinks();
		std::cout << "[RenderGraphEditor] Loaded from: " << file_path << std::endl;
	}
	else
	{
		std::cerr << "[RenderGraphEditor] Load failed: " << Render::RenderGraphSerializer::GetLastError() << std::endl;
		is_executed = false;
	}
}

void OpenGraphCmd::Undo()
{
	if (!is_executed) return;
	is_executed = false;

	auto& nodes = panel->GetNodes();
	auto& links = panel->GetLinks();

	// Clean up loaded state
	for (auto* link : links) { link->Release(); delete link; }
	for (auto* node : nodes) { node->Release(); delete node; }
	nodes.clear();
	links.clear();

	// Restore saved state
	nodes = saved_nodes;
	links = saved_links;
	panel->SetSelectedNode(saved_selection);

	saved_nodes.clear();
	saved_links.clear();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
