#include "UI/RenderGraphEditor/Commands/DeleteNodeCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/BaseLink.h"
#include "UI/EditorItemRegistry.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

DeleteNodeCmd::DeleteNodeCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id)
	: panel(in_panel), node_id(in_node_id)
{
}

void DeleteNodeCmd::Execute()
{
	if (!panel) return;
	if (is_executed) return;

	auto& nodes = panel->GetNodes();
	for (UInt32 i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] && nodes[i]->GetSelfHandle() == node_id.value)
		{
			owned_node = nodes[i];
			saved_position = ImVec2(0, 0);
			nodes.erase(nodes.begin() + i);
			break;
		}
	}

	if (!owned_node) return;

	auto& links = panel->GetLinks();
	Vector<BaseLink*> remaining;
	for (auto* link : links)
	{
		if (!link) continue;
		PinHandle sid = link->GetStartHandle();
		PinHandle eid = link->GetEndHandle();
		Bool connected = false;
		for (auto* pin : owned_node->GetInputPins())
			if (pin && (pin->GetSelfHandle() == sid.value || pin->GetSelfHandle() == eid.value))
				connected = true;
		for (auto* pin : owned_node->GetOutputPins())
			if (pin && (pin->GetSelfHandle() == sid.value || pin->GetSelfHandle() == eid.value))
				connected = true;
		if (connected)
			owned_links.push_back(link);
		else
			remaining.push_back(link);
	}
	links = remaining;

	old_selected = panel->GetSelectedNode();
	if (old_selected == owned_node)
		panel->SetSelectedNode(nullptr);

	is_executed = true;
}

void DeleteNodeCmd::Undo()
{
	if (!panel || !owned_node || !is_executed) return;

	panel->GetNodes().push_back(owned_node);
	owned_node->SetPendingPosition(saved_position.x, saved_position.y);

	for (auto* link : owned_links)
		panel->GetLinks().push_back(link);
	owned_links.clear();

	panel->SetSelectedNode(old_selected);

	is_executed = false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
