#include "UI/RenderGraphEditor/Commands/DeleteNodeCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/BaseLink.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

DeleteNodeCmd::DeleteNodeCmd(RenderGraphPanel* in_panel, UInt64 in_node_id)
	: panel(in_panel), node_id(in_node_id)
{
}

void DeleteNodeCmd::Execute()
{
	if (!panel) return;
	if (is_executed) return;

	// ���ҽڵ�
	auto& nodes = panel->GetNodes();
	for (UInt32 i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i] && nodes[i]->GetSelfID() == node_id)
		{
			owned_node = nodes[i];
			// ʹ�� pending_pos ����ȡǶ��λ�ã����� ed::GetNodePosition �����ȷ
			// ���ڴ� imgui node editor λ�ñ�����ڲ�״̬��
			// ���� RenderGraphPanel::BuildDefinition �л��ȡλ��
			saved_position = ImVec2(0, 0);
			nodes.erase(nodes.begin() + i);
			break;
		}
	}

	if (!owned_node) return;

	// ɾ������������
	auto& links = panel->GetLinks();
	Vector<BaseLink*> remaining;
	for (auto* link : links)
	{
		if (!link) continue;
		UInt64 sid = link->GetStartID();
		UInt64 eid = link->GetEndID();
		Bool connected = false;
		for (auto* pin : owned_node->GetInputPins())
			if (pin && (pin->GetSelfID() == sid || pin->GetSelfID() == eid))
				connected = true;
		for (auto* pin : owned_node->GetOutputPins())
			if (pin && (pin->GetSelfID() == sid || pin->GetSelfID() == eid))
				connected = true;
		if (connected)
			owned_links.push_back(link);
		else
			remaining.push_back(link);
	}
	links = remaining;

	// ��������ѡ��
	old_selected = panel->GetSelectedNode();
	if (old_selected == owned_node)
		panel->SetSelectedNode(nullptr);

	is_executed = true;
}

void DeleteNodeCmd::Undo()
{
	if (!panel || !owned_node || !is_executed) return;

	// �ָ��ڵ㵽 panel
	panel->GetNodes().push_back(owned_node);
	owned_node->SetPendingPosition(saved_position.x, saved_position.y);

	// �ָ���������
	for (auto* link : owned_links)
		panel->GetLinks().push_back(link);
	owned_links.clear();

	// �ָ�ѡ��
	panel->SetSelectedNode(old_selected);

	is_executed = false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
