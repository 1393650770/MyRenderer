// --   --
#include "UI/RenderGraphEditor/Commands/EditorCommandQueue.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

void EditorCommandQueue::Enqueue(std::unique_ptr<Command> cmd)
{
	if (!cmd) return;
	pending.push_back(std::move(cmd));
}

void EditorCommandQueue::ProcessAll(CommandHistory& history)
{
	if (pending.empty()) return;

	//   Swap to avoid iterator invalidation — commands executed here may
	// enqueue additional commands (e.g., LoadDefinition calls AddLinkWithCmd).
	// Those stay in pending for the next frame.
	Vector<std::unique_ptr<Command>> current;
	current.swap(pending);

	for (auto& cmd : current)
	{
		if (!cmd) continue;

		if (cmd->IsUndoable())
		{
			//   通过 CommandHistory 执行，支持 Undo/Redo
			// 如果在 Transaction 中，CommandHistory::Execute 会把 cmd
			// 追加到 active_transaction_ 而非直接推入 undo_stack
			history.Execute(std::move(cmd));
		}
		else
		{
			//   纯 Action（I/O、视图操作等），直接执行不入 history
			cmd->Execute();
		}
	}
}

void EditorCommandQueue::Clear()
{
	pending.clear();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
