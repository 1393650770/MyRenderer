#include "BasePin.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "BaseNode.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)



BasePin::BasePin(PinType in_pin_type, BaseNode* in_owner, CONST String& in_name/*=""*/, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{
	pin_type = in_pin_type;
	owner = in_owner;
}
void BasePin::Draw()
{
	ed::PinId id = self_id;
	ed::BeginPin(id, ed::PinKind(pin_type));
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
	return size;
}

BaseNode* BasePin::GetBelongNode()
{
	return owner;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE