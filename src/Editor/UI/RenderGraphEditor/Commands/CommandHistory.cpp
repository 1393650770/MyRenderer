// -- [AI:BEGIN] --
#include "UI/RenderGraphEditor/Commands/CommandHistory.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

void CommandHistory::Execute(std::unique_ptr<Command> cmd)
{
	if (!cmd) return;

	// Check if we can merge with the previous command
	if (!undo_stack.empty() && cmd->CanMerge(*undo_stack.back()))
	{
		undo_stack.back()->Merge(std::move(cmd));
		return;
	}

	cmd->Execute();
	if (undo_stack.size() >= MAX_DEPTH)
		undo_stack.erase(undo_stack.begin());
	undo_stack.push_back(std::move(cmd));

	// Clear redo stack on new action
	redo_stack.clear();
}

void CommandHistory::Undo()
{
	if (undo_stack.empty()) return;
	auto cmd = std::move(undo_stack.back());
	undo_stack.pop_back();
	cmd->Undo();
	if (redo_stack.size() >= MAX_DEPTH)
		redo_stack.erase(redo_stack.begin());
	redo_stack.push_back(std::move(cmd));
}

void CommandHistory::Redo()
{
	if (redo_stack.empty()) return;
	auto cmd = std::move(redo_stack.back());
	redo_stack.pop_back();
	cmd->Execute();
	undo_stack.push_back(std::move(cmd));
}

void CommandHistory::Clear()
{
	undo_stack.clear();
	redo_stack.clear();
}

Bool CommandHistory::CanUndo() CONST { return !undo_stack.empty(); }
Bool CommandHistory::CanRedo() CONST { return !redo_stack.empty(); }

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// -- [AI:END] --
