#include "UI/RenderGraphEditor/Commands/CreateLinkCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseLink.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

CreateLinkCmd::CreateLinkCmd(RenderGraphPanel* in_panel, BaseLink* in_link)
	: panel(in_panel), link_raw(in_link), is_in_panel(false)
{
}

void CreateLinkCmd::Execute()
{
	if (!link_raw || !panel) return;

	if (!is_in_panel)
	{
		panel->GetLinks().push_back(link_raw);
		is_in_panel = true;
	}
}

void CreateLinkCmd::Undo()
{
	if (!link_raw || !panel) return;

	auto& links = panel->GetLinks();
	for (UInt32 i = 0; i < links.size(); ++i)
	{
		if (links[i] == link_raw)
		{
			links.erase(links.begin() + i);
			break;
		}
	}
	is_in_panel = false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
