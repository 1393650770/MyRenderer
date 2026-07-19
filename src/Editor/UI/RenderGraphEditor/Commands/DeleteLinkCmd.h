#pragma once
#ifndef _RENDERGRAPH_DELETELINKCMD_
#define _RENDERGRAPH_DELETELINKCMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"
#include "UI/EditorItemHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseLink;
class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(DeleteLinkCmd, public Command)
#pragma region METHOD
public:
	DeleteLinkCmd(RenderGraphPanel* in_panel, LinkHandle link_id);
	VIRTUAL ~DeleteLinkCmd() MYDEFAULT;

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	LinkHandle link_id;
	BaseLink* owned_link = nullptr;
	Bool is_executed = false;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
