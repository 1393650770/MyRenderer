#pragma once
#ifndef _RENDERGRAPH_NEWGRAPHCMD_
#define _RENDERGRAPH_NEWGRAPHCMD_

#include "CommandHistory.h"
#include "Core/ConstDefine.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BaseLink;
class RenderGraphPanel;

class NewGraphCmd : public Command
{
public:
	NewGraphCmd(RenderGraphPanel* in_panel);
	virtual ~NewGraphCmd() = default;

	void Execute() override;
	void Undo() override;
	String GetDescription() const override { return "New Graph"; }

private:
	RenderGraphPanel* panel;
	Vector<BaseNode*> saved_nodes;
	Vector<BaseLink*> saved_links;
	BaseNode* saved_selection = nullptr;
	bool is_executed = false;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
