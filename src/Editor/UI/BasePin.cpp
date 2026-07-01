#include "BasePin.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "BaseNode.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)



BasePin::BasePin(PinType in_pin_type, BaseNode* in_owner, PinAccess in_access, CONST String& in_name/*=""*/, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{
	pin_type = in_pin_type;
	owner = in_owner;
	access_type = in_access;
}
void BasePin::Draw()
{
	ed::PinId id = self_id;
	ed::BeginPin(id, ed::PinKind(pin_type));

	// Draw colored circle indicator for pin access type
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
	ImVec2 circle_center = ImVec2(cursor_pos.x + 6, cursor_pos.y + ImGui::GetTextLineHeight() * 0.5f);
	ImColor pin_color = RenderGraphColors::GetPinColor(access_type);
	draw_list->AddCircleFilled(circle_center, 4.0f, pin_color);

	// Indent text to make room for the circle
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12.0f);

	if (pin_type == PinType::Input)
	{
		ImGui::Text(("-> " + name).c_str());
	}
	else
	{
		ImGui::Text((name + " ->").c_str());
	}
	ed::EndPin();
}

void BasePin::Init()
{
}

void BasePin::Release()
{
}
ImVec2 BasePin::GetSize()
{
	ImVec2 size;
	if (pin_type == PinType::Input)
	{
		String text = "-> " + name;
		size= ImGui::CalcTextSize(text.c_str());
	}
	else
	{
		String text = name + " ->";
		size = ImGui::CalcTextSize(text.c_str());
	}
	// Add space for the colored circle indicator
	size.x += 14.0f;
	return size;
}

BaseNode* BasePin::GetBelongNode()
{
	return owner;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
