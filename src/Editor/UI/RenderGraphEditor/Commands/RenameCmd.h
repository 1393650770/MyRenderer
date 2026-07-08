#pragma once
#ifndef _RENDERGRAPH_RENAMECMD_
#define _RENDERGRAPH_RENAMECMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenameCmd, public Command)
#pragma region METHOD
public:
	RenameCmd(RenderGraphPanel* in_panel, UInt64 in_item_id, CONST String& in_old_name, CONST String& in_new_name);

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
	VIRTUAL Bool METHOD(CanMerge)(CONST Command& other) CONST OVERRIDE FINAL;
	VIRTUAL void METHOD(Merge)(std::unique_ptr<Command> other) OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	UInt64 item_id;
	String old_name;
	String new_name;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
