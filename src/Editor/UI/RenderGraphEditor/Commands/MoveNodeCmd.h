#pragma once
#ifndef _RENDERGRAPH_MOVENODECMD_
#define _RENDERGRAPH_MOVENODECMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"
#include "UI/EditorItemHandle.h"
#include "imgui.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(MoveNodeCmd, public Command)
#pragma region METHOD
public:
	MoveNodeCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id, ImVec2 in_old_pos, ImVec2 in_new_pos);

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
	VIRTUAL Bool METHOD(CanMerge)(CONST Command& other) CONST OVERRIDE FINAL;
	VIRTUAL void METHOD(Merge)(std::unique_ptr<Command> other) OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	NodeHandle node_id;
	ImVec2 old_pos;
	ImVec2 new_pos;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
