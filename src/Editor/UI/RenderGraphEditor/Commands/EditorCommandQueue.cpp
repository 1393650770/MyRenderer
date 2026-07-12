// --   --
#include "UI/RenderGraphEditor/Commands/EditorCommandQueue.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

void EditorCommandQueue::Enqueue(std::unique_ptr<Command> cmd)
{
	if (!cmd) return;
	std::lock_guard<std::mutex> lock(mtx);
	pending.push_back(std::move(cmd));
}

void EditorCommandQueue::ProcessAll(CommandHistory& history)
{
	Vector<std::unique_ptr<Command>> current;
	{
		std::lock_guard<std::mutex> lock(mtx);
		if (pending.empty()) return;
		current.swap(pending);
	}

	for (auto& cmd : current)
	{
		if (!cmd) continue;

		if (cmd->IsUndoable())
		{
			history.Execute(std::move(cmd));
		}
		else
		{
			cmd->Execute();
		}
	}
}

void EditorCommandQueue::Clear()
{
	std::lock_guard<std::mutex> lock(mtx);
	pending.clear();
}

size_t EditorCommandQueue::PendingCount() const
{
	std::lock_guard<std::mutex> lock(mtx);
	return pending.size();
}

bool EditorCommandQueue::IsEmpty() const
{
	std::lock_guard<std::mutex> lock(mtx);
	return pending.empty();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
