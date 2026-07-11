#pragma once
#ifndef _RENDERGRAPH_COMMANDHISTORY_
#define _RENDERGRAPH_COMMANDHISTORY_

#include "Core/ConstDefine.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// ----   Command flags — determines how the queue executes the command
enum class ECommandFlags : UInt8
{
	None      = 0,
	Undoable  = 1 << 0,  //   Go through CommandHistory (supports Undo/Redo)
	Immediate = 1 << 1,  //   Execute immediately, don't enqueue (Undo/Redo itself)
};

MYRENDERER_BEGIN_CLASS(Command)
public:
	VIRTUAL ~Command() MYDEFAULT;
	VIRTUAL void METHOD(Execute)() PURE;
	VIRTUAL void METHOD(Undo)() PURE;
	// --   Merge sequential same-type commands (drag operations)
	VIRTUAL Bool METHOD(CanMerge)(CONST Command& other) CONST { return false; }
	VIRTUAL void METHOD(Merge)(std::unique_ptr<Command> other) {}
	// --   Flags: Undoable by default
	VIRTUAL ECommandFlags METHOD(GetFlags)() CONST { return ECommandFlags::Undoable; }
	VIRTUAL String METHOD(GetDescription)() CONST { return ""; }
	Bool METHOD(IsUndoable)()  CONST { return !!(static_cast<UInt8>(GetFlags()) & static_cast<UInt8>(ECommandFlags::Undoable)); }
	Bool METHOD(IsImmediate)() CONST { return !!(static_cast<UInt8>(GetFlags()) & static_cast<UInt8>(ECommandFlags::Immediate)); }
MYRENDERER_END_CLASS

// ----   CompositeCmd — 将多个 Command 合并为一个原子 Undo 步
class CompositeCmd : public Command
{
public:
	CompositeCmd() = default;
	CompositeCmd(const String& desc) : description_(desc) {}
	virtual ~CompositeCmd() = default;

	void Execute() override
	{
		for (auto& cmd : sub_commands)
			if (cmd) cmd->Execute();
	}

	void Undo() override
	{
		for (auto it = sub_commands.rbegin(); it != sub_commands.rend(); ++it)
			if (*it) (*it)->Undo();
	}

	String GetDescription() const override { return description_; }
	ECommandFlags GetFlags() const override { return ECommandFlags::Undoable; }

	void AddCommand(std::unique_ptr<Command> cmd)
	{
		if (cmd) sub_commands.push_back(std::move(cmd));
	}

	bool IsEmpty() const { return sub_commands.empty(); }
	size_t Count() const { return sub_commands.size(); }

private:
	Vector<std::unique_ptr<Command>> sub_commands;
	String description_;
};

MYRENDERER_BEGIN_CLASS(CommandHistory)
#pragma region METHOD
public:
	CommandHistory() MYDEFAULT;
	~CommandHistory();

	void METHOD(Execute)(std::unique_ptr<Command> cmd);
	void METHOD(Undo)();
	void METHOD(Redo)();
	void METHOD(Clear)();
	Bool METHOD(CanUndo)() CONST;
	Bool METHOD(CanRedo)() CONST;

	// ----   Transaction grouping — multiple Commands merged into one Undo step
	// 用法示例（在 Draw 中）:
	//   BEGIN_TXN("Paste Nodes");
	//   QUEUE_CMD(CreateNodeCmd, ...);
	//   QUEUE_CMD(CreateLinkCmd, ...);
	//   END_TXN();
	// Ctrl+Z 时一次性撤销所有 CreateNode/CreateLink 命令
	void METHOD(BeginTransaction)(CONST String& desc);
	void METHOD(EndTransaction)();
	void METHOD(CancelTransaction)();
	Bool METHOD(IsInTransaction)() CONST { return active_transaction != nullptr; }
	CompositeCmd* METHOD(GetActiveTransaction)() { return active_transaction.get(); }

private:
	static constexpr UInt32 MAX_DEPTH = 256;
#pragma endregion

#pragma region MEMBER
protected:
	Vector<std::unique_ptr<Command>> undo_stack;
	Vector<std::unique_ptr<Command>> redo_stack;
	std::unique_ptr<CompositeCmd> active_transaction; //   Current transaction being built
private:
#pragma endregion
MYRENDERER_END_CLASS

// ----   Transaction macros — use inside RenderGraphPanel member functions
// BEGIN_TXN opens a transaction; END_TXN closes it. All QUEUE_CMD calls between
// them become sub-commands of a single CompositeCmd in the undo stack.

// Internal marker commands for transaction begin/end (don't use directly)
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(TransactionBeginCmd, public Command)
public:
	TransactionBeginCmd(CommandHistory* h, String desc)
		: history(h), desc_(std::move(desc)) {}
	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL { history->BeginTransaction(desc_); }
	VIRTUAL void METHOD(Undo)()    OVERRIDE FINAL {}
	VIRTUAL ECommandFlags METHOD(GetFlags)() CONST OVERRIDE FINAL { return ECommandFlags::None; }
	VIRTUAL String METHOD(GetDescription)() CONST OVERRIDE FINAL { return desc_; }
private:
	CommandHistory* history;
	String desc_;
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(TransactionEndCmd, public Command)
public:
	TransactionEndCmd(CommandHistory* h) : history(h) {}
	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL { history->EndTransaction(); }
	VIRTUAL void METHOD(Undo)()    OVERRIDE FINAL {}
	VIRTUAL ECommandFlags METHOD(GetFlags)() CONST OVERRIDE FINAL { return ECommandFlags::None; }
private:
	CommandHistory* history;
MYRENDERER_END_CLASS

//   Transaction macros — use inside RenderGraphPanel member functions
#define BEGIN_TXN(desc) \
	cmd_queue.Enqueue(std::make_unique<UI::TransactionBeginCmd>(&command_history, desc))

#define END_TXN() \
	cmd_queue.Enqueue(std::make_unique<UI::TransactionEndCmd>(&command_history))

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPH_COMMANDHISTORY_
