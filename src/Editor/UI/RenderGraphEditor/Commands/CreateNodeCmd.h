#pragma once
#ifndef _RENDERGRAPH_CREATENODECMD_
#define _RENDERGRAPH_CREATENODECMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"
#include "imgui.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CreateNodeCmd, public Command)
#pragma region METHOD
public:
	CreateNodeCmd(RenderGraphPanel* in_panel, BaseNode* in_node, ImVec2 in_pos);
	VIRTUAL ~CreateNodeCmd() MYDEFAULT;

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	BaseNode* node_raw = nullptr;       // panel��nodes �е�raw pointer
	ImVec2 position;
	Bool is_in_panel = false;           // �ڵ��Ƿ��ڵ�ǰ��panel��
	Vector<UInt64> owned_link_ids;      // undoʱ��Ҫɾ����������
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
