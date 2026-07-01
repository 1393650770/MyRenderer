#include "RenderGraphSubGraphNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"
#include "UI/BasePin.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include <algorithm>
#include <cfloat>

namespace ed = ax::NodeEditor;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

RenderGraphSubGraphNode::RenderGraphSubGraphNode(CONST String& in_name, Bool in_show)
	: BaseNode(in_name, in_show)
{
}

void RenderGraphSubGraphNode::Draw()
{
	if (m_collapsed)
	{
		DrawCollapsed();
		return;
	}
	DrawExpandedHeader();
}

void RenderGraphSubGraphNode::Release()
{
	m_children.clear();
	m_exposed_inputs.clear();
	m_exposed_outputs.clear();
	BaseNode::Release();
}

void RenderGraphSubGraphNode::AddChildPass(RenderGraphPassNode* pass)
{
	if (pass) m_children.push_back(pass);
}

void RenderGraphSubGraphNode::AddChildResource(RenderGraphResourceNode* resource)
{
	if (resource) m_children.push_back(resource);
}

void RenderGraphSubGraphNode::RemoveChild(UInt64 node_id)
{
	for (auto it = m_children.begin(); it != m_children.end(); ++it)
	{
		if (*it && (*it)->GetSelfID() == node_id)
		{
			m_children.erase(it);
			return;
		}
	}
}

void RenderGraphSubGraphNode::ExposeInput(CONST String& exposed_name, BaseNode* child, CONST String& child_pin_name)
{
	// Add a pin to the subgraph node's own input list
	BaseNode::AddInput(exposed_name);
	m_exposed_inputs.push_back({ child ? child->GetSelfID() : (UInt64)0, child_pin_name });
}

void RenderGraphSubGraphNode::ExposeOutput(CONST String& exposed_name, BaseNode* child, CONST String& child_pin_name)
{
	BaseNode::AddOutput(exposed_name);
	m_exposed_outputs.push_back({ child ? child->GetSelfID() : (UInt64)0, child_pin_name });
}

void RenderGraphSubGraphNode::SetCollapsed(Bool collapsed)
{
	m_collapsed = collapsed;
	SetSetNeedRecalcSize();
}

Bool RenderGraphSubGraphNode::IsSubGraphNode(BaseNode* node)
{
	return dynamic_cast<RenderGraphSubGraphNode*>(node) != nullptr;
}

void RenderGraphSubGraphNode::DrawCollapsed()
{
	ed::BeginNode(GetSelfID());
	ImGui::PushID((Int)GetSelfID());

	ImGui::TextColored(ImColor(180, 180, 220), "[SG]");
	ImGui::SameLine();
	if (ImGui::Button("+")) SetCollapsed(false);
	ImGui::SameLine();
	ImGui::Text("%s", GetName().c_str());

	for (auto* pin : GetInputPins()) pin->Draw();
	for (auto* pin : GetOutputPins()) pin->Draw();

	ImGui::PopID();
	ed::EndNode();
}

void RenderGraphSubGraphNode::DrawExpandedHeader()
{
	if (m_children.empty()) return;

	ImVec2 min_pos(FLT_MAX, FLT_MAX);
	ImVec2 max_pos(-FLT_MAX, -FLT_MAX);
	for (auto* child : m_children)
	{
		if (!child) continue;
		ImVec2 pos = ed::GetNodePosition(ed::NodeId(child->GetSelfID()));
		min_pos.x = (std::min)(min_pos.x, pos.x);
		min_pos.y = (std::min)(min_pos.y, pos.y);
		max_pos.x = (std::max)(max_pos.x, pos.x + 200.0f);
		max_pos.y = (std::max)(max_pos.y, pos.y + 100.0f);
	}

	ImDrawList* dl = ed::GetNodeBackgroundDrawList(ed::NodeId(GetSelfID()));
	if (dl && min_pos.x != FLT_MAX)
	{
		ImVec2 padding(10, 30);
		dl->AddRectFilled(min_pos - padding, max_pos + padding, m_color, 4.0f);
		dl->AddRect(min_pos - padding, max_pos + padding, IM_COL32(120, 120, 180, 200), 4.0f);

		ImVec2 btn_pos(min_pos.x - padding.x + 5, min_pos.y - padding.y + 5);
		dl->AddText(btn_pos, IM_COL32_WHITE, "[-]");
	}

	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	{
		ImVec2 mouse = ImGui::GetMousePos();
		if (mouse.x >= min_pos.x - 10 && mouse.y >= min_pos.y - 30
			&& mouse.x <= max_pos.x + 10 && mouse.y <= min_pos.y)
			SetCollapsed(true);
	}
}

void RenderGraphSubGraphNode::RecalcSize()
{
	BaseNode::RecalcSize();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
