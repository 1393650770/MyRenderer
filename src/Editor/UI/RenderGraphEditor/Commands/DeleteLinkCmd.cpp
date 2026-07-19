#include "UI/RenderGraphEditor/Commands/DeleteLinkCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseLink.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

DeleteLinkCmd::DeleteLinkCmd(RenderGraphPanel* in_panel, LinkHandle in_link_id)
	: panel(in_panel), link_id(in_link_id)
{
}

void DeleteLinkCmd::Execute()
{
	if (!panel || is_executed) return;

	auto& links = panel->GetLinks();
	for (UInt32 i = 0; i < links.size(); ++i)
	{
		if (links[i] && links[i]->GetSelfHandle() == link_id.value)
		{
			owned_link = links[i];
			links.erase(links.begin() + i);
			is_executed = true;
			return;
		}
	}
}

void DeleteLinkCmd::Undo()
{
	if (!panel || !owned_link || !is_executed) return;

	panel->GetLinks().push_back(owned_link);
	is_executed = false;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
