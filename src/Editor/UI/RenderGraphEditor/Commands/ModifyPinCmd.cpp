#include "UI/RenderGraphEditor/Commands/ModifyPinCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

ModifyPinCmd::ModifyPinCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id, EAction in_action)
	: panel(in_panel), node_id(in_node_id), action(in_action)
{
}

ModifyPinCmd::ModifyPinCmd(RenderGraphPanel* in_panel, NodeHandle in_node_id, PinHandle in_pin_id, EAction in_action, const String& in_new_name)
	: panel(in_panel), node_id(in_node_id), pin_id(in_pin_id), action(in_action), new_name(in_new_name)
{
}

void ModifyPinCmd::Execute()
{
	if (is_executed) return;
	is_executed = true;

	auto* node = panel->GetNodeByHandle(node_id);
	if (!node) return;

	switch (action)
	{
	case EAction::AddInput:
		if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
			pass->AddInputPin("Input", PinAccess::Read);
		else
			node->AddInput("Input");
		break;

	case EAction::AddOutput:
		if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
			pass->AddOutputPin("Output", PinAccess::Write);
		else
			node->AddOutput("Output");
		break;

	case EAction::Delete:
		if (auto* pin = node->GetPin(pin_id))
		{
			old_name = pin->GetName();
			owned_pin = pin;
			node->DeletePin(pin_id);
		}
		break;

	case EAction::Rename:
		if (auto* pin = node->GetPin(pin_id))
		{
			old_name = pin->GetName();
			pin->SetName(new_name);
			node->SetSetNeedRecalcSize();
		}
		break;
	}
}

void ModifyPinCmd::Undo()
{
	if (!is_executed) return;
	is_executed = false;

	auto* node = panel->GetNodeByHandle(node_id);
	if (!node) return;

	switch (action)
	{
	case EAction::AddInput:
		if (!node->GetInputPins().empty())
			node->DeletePin(PinHandle{ node->GetInputPins().back()->GetSelfHandle() });
		break;

	case EAction::AddOutput:
		if (!node->GetOutputPins().empty())
			node->DeletePin(PinHandle{ node->GetOutputPins().back()->GetSelfHandle() });
		break;

	case EAction::Delete:
		if (owned_pin)
		{
			node->AddInput(old_name);
		}
		break;

	case EAction::Rename:
		if (auto* pin = node->GetPin(pin_id))
		{
			pin->SetName(old_name);
			node->SetSetNeedRecalcSize();
		}
		break;
	}
}

String ModifyPinCmd::GetDescription() const
{
	switch (action)
	{
	case EAction::AddInput:  return "Add Input Pin";
	case EAction::AddOutput: return "Add Output Pin";
	case EAction::Delete:    return "Delete Pin";
	case EAction::Rename:    return "Rename Pin";
	default: return "Modify Pin";
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
