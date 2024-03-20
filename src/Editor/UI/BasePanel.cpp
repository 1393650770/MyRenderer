#include "BasePanel.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)



BasePanel::BasePanel(CONST String& in_name, Bool in_show /*= true*/) :name(in_name), is_show(in_show)
{
}

Bool BasePanel::OnBegin(Int window_flags)
{
	if (!is_show)
		return false;

	ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);

	ImGui::Begin(name.c_str(), &is_show, window_flags | ImGuiWindowFlags_NoCollapse);
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(4.0f,2.0f));
	return true;
}

void BasePanel::OnEnd() CONST
{
	//ImGui::PopStyleVar();
	ImGui::End();
}

CONST String& BasePanel::GetName() CONST
{
	return name;
}

BasePanel* BasePanel::CreatePanel(CONST String& in_name, Bool in_show /*= true*/)
{
	return panel_createfunc_map[in_name](in_name, in_show);
}

Map<String, MXRender::UI::PanelCreateFunction> BasePanel::panel_createfunc_map;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE