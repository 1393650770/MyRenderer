#include "BasePin.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)



UInt64 BasePin::pin_id=1;



BasePin::BasePin( PinType in_pin_type /*= PinType::Input*/, CONST String& in_name/*=""*/, Bool in_show /*= true*/) :  pin_type(in_pin_type), name(in_name), is_show(in_show)
{
	self_id = pin_id++;
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
		size= ImGui::CalcTextSize(("-> " + name).c_str());
	}
	else
	{
		size = ImGui::CalcTextSize((name + " ->").c_str());
	}
	return size;
}

UInt64 BasePin::GetSelfID() CONST
{
	return self_id;
}


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE