#pragma once
#ifndef _RENDERGRAPH_CREATELINKCMD_
#define _RENDERGRAPH_CREATELINKCMD_

#include "UI/RenderGraphEditor/Commands/CommandHistory.h"
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseLink;
class RenderGraphPanel;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(CreateLinkCmd, public Command)
#pragma region METHOD
public:
	CreateLinkCmd(RenderGraphPanel* in_panel, BaseLink* in_link);
	VIRTUAL ~CreateLinkCmd() MYDEFAULT;

	VIRTUAL void METHOD(Execute)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Undo)() OVERRIDE FINAL;
#pragma endregion

#pragma region MEMBER
private:
	RenderGraphPanel* panel;
	BaseLink* link_raw = nullptr;
	Bool is_in_panel = false;
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
