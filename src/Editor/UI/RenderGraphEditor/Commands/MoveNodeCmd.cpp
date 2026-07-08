#include "UI/RenderGraphEditor/Commands/MoveNodeCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"

namespace ed = ax::NodeEditor;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MoveNodeCmd::MoveNodeCmd(RenderGraphPanel* in_panel, UInt64 in_node_id, ImVec2 in_old_pos, ImVec2 in_new_pos)
	: panel(in_panel), node_id(in_node_id), old_pos(in_old_pos), new_pos(in_new_pos)
{
}

void MoveNodeCmd::Execute()
{
	ed::SetNodePosition(node_id, new_pos);
}

void MoveNodeCmd::Undo()
{
	ed::SetNodePosition(node_id, old_pos);
}

Bool MoveNodeCmd::CanMerge(CONST Command& other) CONST
{
	auto* r = dynamic_cast<CONST MoveNodeCmd*>(&other);
	return r && r->node_id == this->node_id;
}

void MoveNodeCmd::Merge(std::unique_ptr<Command> other)
{
	auto* r = dynamic_cast<MoveNodeCmd*>(other.get());
	if (r)
		new_pos = r->new_pos;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
