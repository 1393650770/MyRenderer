#pragma once
#ifndef _RENDERGRAPH_COMMANDHISTORY_
#define _RENDERGRAPH_COMMANDHISTORY_

#include "Core/ConstDefine.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(Command)
public:
	VIRTUAL ~Command() MYDEFAULT;
	VIRTUAL void METHOD(Execute)() PURE;
	VIRTUAL void METHOD(Undo)() PURE;
	// -- [AI] Merge sequential same-type commands (drag operations)
	VIRTUAL Bool METHOD(CanMerge)(CONST Command& other) CONST { return false; }
	VIRTUAL void METHOD(Merge)(std::unique_ptr<Command> other) {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(CommandHistory)
#pragma region METHOD
public:
	CommandHistory() MYDEFAULT;
	~CommandHistory() MYDEFAULT;

	void METHOD(Execute)(std::unique_ptr<Command> cmd);
	void METHOD(Undo)();
	void METHOD(Redo)();
	void METHOD(Clear)();
	Bool METHOD(CanUndo)() CONST;
	Bool METHOD(CanRedo)() CONST;
private:
	static constexpr UInt32 MAX_DEPTH = 256;
#pragma endregion

#pragma region MEMBER
protected:
	Vector<std::unique_ptr<Command>> undo_stack;
	Vector<std::unique_ptr<Command>> redo_stack;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPH_COMMANDHISTORY_
