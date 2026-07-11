// --   --
#include "UI/RenderGraphEditor/Commands/CommandHistory.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

CommandHistory::~CommandHistory()
{
	//   Need to be defined in .cpp because std::unique_ptr<CompositeCmd>
	// requires the complete type for destruction.
}

void CommandHistory::Execute(std::unique_ptr<Command> cmd)
{
	if (!cmd) return;

	//   If we're inside a Transaction, append to active_transaction_ instead of
	// pushing to undo_stack directly. EndTransaction() will push the composite.
	if (active_transaction)
	{
		active_transaction->AddCommand(std::move(cmd));
		return;
	}

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
	active_transaction.reset();
}

Bool CommandHistory::CanUndo() CONST { return !undo_stack.empty(); }
Bool CommandHistory::CanRedo() CONST { return !redo_stack.empty(); }

// ----   Transaction support ----

void CommandHistory::BeginTransaction(CONST String& desc)
{
	// If a transaction is already active, it's a nested call — ignore.
	// We don't support nested transactions.
	if (active_transaction) return;

	active_transaction = std::make_unique<CompositeCmd>(desc);
}

void CommandHistory::EndTransaction()
{
	if (!active_transaction) return;

	// Push the composite as a single undo step
	auto txn = std::move(active_transaction);
	active_transaction = nullptr;

	// Only push if it has sub-commands
	if (txn->IsEmpty()) return;

	undo_stack.push_back(std::move(txn));
	if (undo_stack.size() > MAX_DEPTH)
		undo_stack.erase(undo_stack.begin());
	redo_stack.clear();
}

void CommandHistory::CancelTransaction()
{
	active_transaction.reset();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
