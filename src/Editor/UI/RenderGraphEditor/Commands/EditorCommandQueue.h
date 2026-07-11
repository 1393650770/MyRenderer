#pragma once
#ifndef _RENDERGRAPH_EDITORCOMMANDQUEUE_
#define _RENDERGRAPH_EDITORCOMMANDQUEUE_

#include "Core/ConstDefine.h"
#include "CommandHistory.h"
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// ----   LambdaCmd — wraps Execute/Undo lambdas
class LambdaCmd : public Command
{
public:
	using Fn = std::function<void()>;

	LambdaCmd(String desc, Fn exec, Fn undo)
		: desc_(std::move(desc)), exec_(std::move(exec)), undo_(std::move(undo)) {}

	LambdaCmd(String desc, Fn exec)
		: desc_(std::move(desc)), exec_(std::move(exec)) {}

	void Execute() override { if (exec_) exec_(); }
	void Undo()    override { if (undo_) undo_(); }
	String GetDescription() const override { return desc_; }
	ECommandFlags GetFlags() const override
	{
		return undo_ ? ECommandFlags::Undoable : ECommandFlags::None;
	}

private:
	String desc_;
	Fn exec_;
	Fn undo_;
};

// ----   EditorCommandQueue
class EditorCommandQueue
{
public:
	EditorCommandQueue() = default;
	~EditorCommandQueue() = default;

	void Enqueue(std::unique_ptr<Command> cmd);
	void ProcessAll(CommandHistory& history);
	void Clear();
	size_t PendingCount() const { return pending.size(); }
	bool IsEmpty() const { return pending.empty(); }

private:
	Vector<std::unique_ptr<Command>> pending;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

// ============================================================
// 入队宏 — 在 RenderGraphPanel 成员函数内使用
// 定义在全局作用域，确保在任何 namespace 内都能展开
// ============================================================

// 宏 1: QUEUE_CMD — 专门 Command 子类（复杂撤销逻辑）
#define QUEUE_CMD(CmdType, ...) \
	cmd_queue.Enqueue(std::make_unique<CmdType>(this, ##__VA_ARGS__))

// 宏 2: QUEUE_LAMBDA — lambda 写执行/撤销
#define QUEUE_LAMBDA(desc, exec_body, undo_body) \
	cmd_queue.Enqueue(std::make_unique<MXRender::UI::LambdaCmd>( \
		desc, \
		[=]() { exec_body; }, \
		[=]() { undo_body; }))

// 宏 3: QUEUE_ACTION — 无 Undo 的纯操作
#define QUEUE_ACTION(desc, ...) \
	cmd_queue.Enqueue(std::make_unique<MXRender::UI::LambdaCmd>( \
		desc, \
		[=]() { __VA_ARGS__; }))

#endif // !_RENDERGRAPH_EDITORCOMMANDQUEUE_
