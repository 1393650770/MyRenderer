#include "RenderGraphConnectionValidator.h"
#include "UI/BasePin.h"
#include "UI/BaseNode.h"
#include "RenderGraphPassNode.h"
#include "RenderGraphResourceNode.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

String RenderGraphConnectionValidator::s_last_error;

ConnectionResult RenderGraphConnectionValidator::Validate(BasePin* start_pin, BasePin* end_pin)
{
	ConnectionResult result;
	s_last_error.clear();

	if (!start_pin || !end_pin)
	{
		result.error_message = "x Invalid pin";
		s_last_error = result.error_message;
		return result;
	}

	BaseNode* start_node = start_pin->GetBelongNode();
	BaseNode* end_node = end_pin->GetBelongNode();

	if (!start_node || !end_node)
	{
		result.error_message = "x Pin has no owner node";
		s_last_error = result.error_message;
		return result;
	}

	// Rule 1: Cannot connect a pin to itself
	if (start_pin == end_pin)
	{
		result.error_message = "x Cannot connect pin to itself";
		s_last_error = result.error_message;
		return result;
	}

	// Rule 2: Cannot connect within the same node
	if (start_node == end_node)
	{
		result.error_message = "x Cannot connect pins on same node";
		s_last_error = result.error_message;
		return result;
	}

	// Rule 3: Pins must be of opposite direction (input <-> output)
	if (start_pin->GetPinType() == end_pin->GetPinType())
	{
		result.error_message = "x Both pins are same direction (need Input <-> Output)";
		s_last_error = result.error_message;
		return result;
	}

	// Rule 4: Must be Pass <-> Resource (not Pass-Pass or Resource-Resource)
	Bool start_is_pass = IsPassNode(start_node);
	Bool end_is_pass = IsPassNode(end_node);
	Bool start_is_res = IsResourceNode(start_node);
	Bool end_is_res = IsResourceNode(end_node);

	if (start_is_pass && end_is_pass)
	{
		result.error_message = "x Pass-Pass direct connection not allowed (must go through Resource)";
		s_last_error = result.error_message;
		return result;
	}

	if (start_is_res && end_is_res)
	{
		result.error_message = "x Resource-Resource direct connection not allowed (must go through Pass)";
		s_last_error = result.error_message;
		return result;
	}

	if ((start_is_pass && end_is_res) || (start_is_res && end_is_pass))
	{
		// Determine which pin is the pass pin and which is the resource pin
		BasePin* pass_pin = start_is_pass ? start_pin : end_pin;
		BasePin* res_pin = start_is_pass ? end_pin : start_pin;

		// Pass Output -> Resource Input: Write/Create
		// Resource Output -> Pass Input: Read
		if (pass_pin->GetPinType() == PinType::Output && res_pin->GetPinType() == PinType::Input)
		{
			// Pass is writing/creating the resource
			result.is_valid = true;
			return result;
		}
		if (res_pin->GetPinType() == PinType::Output && pass_pin->GetPinType() == PinType::Input)
		{
			// Pass is reading the resource
			result.is_valid = true;
			return result;
		}

		result.error_message = "x Invalid pin direction for Pass-Resource connection";
		s_last_error = result.error_message;
		return result;
	}

	result.error_message = "x Unknown node type combination";
	s_last_error = result.error_message;
	return result;
}

Bool RenderGraphConnectionValidator::CanConnect(BasePin* start_pin, BasePin* end_pin)
{
	return Validate(start_pin, end_pin).is_valid;
}

String RenderGraphConnectionValidator::GetLastError()
{
	return s_last_error;
}

Bool RenderGraphConnectionValidator::IsPassNode(BaseNode* node)
{
	return dynamic_cast<RenderGraphPassNode*>(node) != nullptr;
}

Bool RenderGraphConnectionValidator::IsResourceNode(BaseNode* node)
{
	return dynamic_cast<RenderGraphResourceNode*>(node) != nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
