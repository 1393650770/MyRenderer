#include "UI/RenderGraphEditor/Commands/RenameCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/BaseItem.h"
#include "UI/EditorItemRegistry.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

RenameCmd::RenameCmd(RenderGraphPanel* in_panel, GenericHandle in_item_id, CONST String& in_old_name, CONST String& in_new_name)
	: panel(in_panel), item_id(in_item_id), old_name(in_old_name), new_name(in_new_name)
{
}

void RenameCmd::Execute()
{
	if (!panel) return;

	BaseItem* item = GetEditorRegistry()->ResolveItem(item_id);
	if (item)
	{
		item->SetName(new_name);
		BaseNode* node = item->AsNode();
		if (node) node->SetSetNeedRecalcSize();
	}
}

void RenameCmd::Undo()
{
	if (!panel) return;

	BaseItem* item = GetEditorRegistry()->ResolveItem(item_id);
	if (item)
	{
		item->SetName(old_name);
		BaseNode* node = item->AsNode();
		if (node) node->SetSetNeedRecalcSize();
	}
}

Bool RenameCmd::CanMerge(CONST Command& other) CONST
{
	auto* r = dynamic_cast<CONST RenameCmd*>(&other);
	return r && r->item_id == this->item_id;
}

void RenameCmd::Merge(std::unique_ptr<Command> other)
{
	auto* r = dynamic_cast<RenameCmd*>(other.get());
	if (r)
		new_name = r->new_name;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
