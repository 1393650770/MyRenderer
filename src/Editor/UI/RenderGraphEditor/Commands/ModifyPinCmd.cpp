#include "UI/RenderGraphEditor/Commands/ModifyPinCmd.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

ModifyPinCmd::ModifyPinCmd(RenderGraphPanel* in_panel, UInt64 in_node_id, EAction in_action)
	: panel(in_panel), node_id(in_node_id), action(in_action)
{
}

ModifyPinCmd::ModifyPinCmd(RenderGraphPanel* in_panel, UInt64 in_node_id, UInt64 in_pin_id, EAction in_action, const String& in_new_name)
	: panel(in_panel), node_id(in_node_id), pin_id(in_pin_id), action(in_action), new_name(in_new_name)
{
}

void ModifyPinCmd::Execute()
{
	if (is_executed) return;
	is_executed = true;

	auto* node = panel->GetNode(node_id);
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

	auto* node = panel->GetNode(node_id);
	if (!node) return;

	switch (action)
	{
	case EAction::AddInput:
		// Remove the last input pin
		if (!node->GetInputPins().empty())
			node->DeletePin(node->GetInputPins().back()->GetSelfID());
		break;

	case EAction::AddOutput:
		if (!node->GetOutputPins().empty())
			node->DeletePin(node->GetOutputPins().back()->GetSelfID());
		break;

	case EAction::Delete:
		// Restore the deleted pin (simplified: not perfect for all node types)
		if (owned_pin)
		{
			// Pin was deleted, cannot fully restore in current architecture
			// Add a placeholder pin with the old name
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
