#pragma once
#ifndef _RENDERGRAPH_DELETENODECMD_
#define _RENDERGRAPH_DELETENODECMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"
#include "UI/EditorItemHandle.h"
#include "imgui.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BaseLink;
class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(DeleteNodeCmd, public Command)
#pragma region METHOD
public:
	DeleteNodeCmd(RenderGraphPanel* in_panel, NodeHandle node_id);
	VIRTUAL ~DeleteNodeCmd() MYDEFAULT;

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	NodeHandle node_id;
	BaseNode* owned_node = nullptr;                  // ������Ȩ
	Vector<BaseLink*> owned_links;                   // ����������
	ImVec2 saved_position;
	BaseNode* old_selected = nullptr;
	Bool is_executed = false;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
