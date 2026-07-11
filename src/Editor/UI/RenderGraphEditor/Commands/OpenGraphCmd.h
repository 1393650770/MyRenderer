#pragma once
#ifndef _RENDERGRAPH_OPENGRAPHCMD_
#define _RENDERGRAPH_OPENGRAPHCMD_

#include "CommandHistory.h"
#include "Core/ConstDefine.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BaseLink;
class RenderGraphPanel;

class OpenGraphCmd : public Command
{
public:
	OpenGraphCmd(RenderGraphPanel* in_panel, const String& in_path);
	virtual ~OpenGraphCmd() = default;

	void Execute() override;
	void Undo() override;
	String GetDescription() const override { return "Open Graph"; }

private:
	RenderGraphPanel* panel;
	String file_path;
	Vector<BaseNode*> saved_nodes;
	Vector<BaseLink*> saved_links;
	BaseNode* saved_selection = nullptr;
	Vector<BaseNode*> loaded_nodes;
	Vector<BaseLink*> loaded_links;
	bool is_executed = false;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
