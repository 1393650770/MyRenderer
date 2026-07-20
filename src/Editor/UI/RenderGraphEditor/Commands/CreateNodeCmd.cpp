#include "UI/RenderGraphEditor/Commands/CreateNodeCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/BaseLink.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

CreateNodeCmd::CreateNodeCmd(RenderGraphPanel* in_panel, BaseNode* in_node, ImVec2 in_pos)
	: panel(in_panel), node_raw(in_node), position(in_pos), is_in_panel(false)
{
}

void CreateNodeCmd::Execute()
{
	if (!node_raw || !panel) return;

	if (!is_in_panel)
	{
		panel->GetNodes().push_back(node_raw);
		is_in_panel = true;
	}

	node_raw->SetPendingPosition(position.x, position.y);
}

void CreateNodeCmd::Undo()
{
	if (!node_raw || !panel) return;

	owned_link_ids.clear();
	auto& links = panel->GetLinks();
	Vector<BaseLink*> remaining;
	for (auto* link : links)
	{
		if (!link) continue;
		PinHandle sid = link->GetStartHandle();
		PinHandle eid = link->GetEndHandle();
		Bool connected = false;
		for (auto* pin : node_raw->GetInputPins())
			if (pin && (pin->GetSelfHandle() == sid.value || pin->GetSelfHandle() == eid.value))
				connected = true;
		for (auto* pin : node_raw->GetOutputPins())
			if (pin && (pin->GetSelfHandle() == sid.value || pin->GetSelfHandle() == eid.value))
				connected = true;
		if (connected)
			owned_link_ids.push_back(LinkHandle{ link->GetSelfHandle() });
		else
			remaining.push_back(link);
	}
	links = remaining;

	auto& nodes = panel->GetNodes();
	for (UInt32 i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] == node_raw)
		{
			nodes.erase(nodes.begin() + i);
			break;
		}
	}
	is_in_panel = false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
