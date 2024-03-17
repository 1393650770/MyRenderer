#include "RenderGraphPanel.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


RenderGraphPanel::RenderGraphPanel(CONST String& in_name /*= "RenderGraphEditorPannel"*/, Bool in_show /*= true*/) : BasePanel(in_name, in_show)
{

}

void RenderGraphPanel::Init()
{
}

void RenderGraphPanel::Update()
{
}

void RenderGraphPanel::Draw()
{
	constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoScrollbar;
	if (OnBegin(windowFlags))
	{
		ImGui::Text("RenderGraphPanel");
		OnEnd();
	}
}


void RenderGraphPanel::Release()
{
}

CONST String RenderGraphPanel::GetTypeName()
{
	return "RenderGraphPanel";
}

PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE