#pragma once
#ifndef _RENDERGRAPH_MODIFYPINCMD_
#define _RENDERGRAPH_MODIFYPINCMD_

#include "CommandHistory.h"
#include "Core/ConstDefine.h"
#include "UI/EditorItemHandle.h"
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BasePin;
class RenderGraphPanel;

class ModifyPinCmd : public Command
{
public:
	enum class EAction : UInt8
	{
		AddInput,
		AddOutput,
		Delete,
		Rename,
	};

	ModifyPinCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id, EAction action);
	ModifyPinCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id, PinHandle in_pin_id, EAction action, const String& new_name = "");
	virtual ~ModifyPinCmd() = default;

	void Execute() override;
	void Undo() override;
	String GetDescription() const override;

private:
	RenderGraphPanel* panel;
	NodeHandle node_id;
	PinHandle pin_id;
	EAction action;
	String new_name;
	String old_name;
	bool is_executed = false;
	BasePin* owned_pin = nullptr;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
