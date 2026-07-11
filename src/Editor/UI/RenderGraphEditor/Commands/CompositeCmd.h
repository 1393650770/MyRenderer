#pragma once
#ifndef _RENDERGRAPH_COMPOSITECMD_
#define _RENDERGRAPH_COMPOSITECMD_

#include "Core/ConstDefine.h"
#include "CommandHistory.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// ----   CompositeCmd — 将多个 Command 合并为一个原子 Undo 步
// 由 Transaction 系统自动使用，也可以手动构建
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CompositeCmd, public Command)
#pragma region METHOD
public:
	CompositeCmd() MYDEFAULT;
	CompositeCmd(CONST String& desc) : description_(desc) {}
	VIRTUAL ~CompositeCmd() MYDEFAULT;

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL
	{
		for (auto& cmd : sub_commands)
		{
			if (cmd) cmd->Execute();
		}
	}

	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL
	{
		//   Undo in reverse order
		for (auto it = sub_commands.rbegin(); it != sub_commands.rend(); ++it)
		{
			if (*it) (*it)->Undo();
		}
	}

	VIRTUAL String METHOD(GetDescription)() CONST OVERRIDE FINAL { return description_; }
	VIRTUAL ECommandFlags METHOD(GetFlags)() CONST OVERRIDE FINAL { return ECommandFlags::Undoable; }

	void METHOD(AddCommand)(std::unique_ptr<Command> cmd)
	{
		if (cmd) sub_commands.push_back(std::move(cmd));
	}

	Bool METHOD(IsEmpty)() CONST { return sub_commands.empty(); }
	size_t METHOD(Count)() CONST { return sub_commands.size(); }

protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	Vector<std::unique_ptr<Command>> sub_commands;
	String description_;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPH_COMPOSITECMD_
