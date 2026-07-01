#pragma once
#ifndef _RENDERGRAPHCONNECTIONVALIDATOR_
#define _RENDERGRAPHCONNECTIONVALIDATOR_

#include "Core/ConstDefine.h"
#include "RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BasePin;
class BaseNode;
class RenderGraphPassNode;
class RenderGraphResourceNode;

// Result of connection validation
struct ConnectionResult
{
	Bool is_valid = false;
	String error_message;
};

// Validates connections between pins in the RenderGraph editor.
// Enforces the rule: Pass <-> Resource only (no Pass-Pass or Resource-Resource direct connections).
MYRENDERER_BEGIN_CLASS(RenderGraphConnectionValidator)

#pragma region METHOD
public:
	// Check if two pins can be connected. start_pin is the drag source, end_pin is the drop target.
	static ConnectionResult METHOD(Validate)(BasePin* start_pin, BasePin* end_pin);

	// Convenience: validate and return just the boolean result
	static Bool METHOD(CanConnect)(BasePin* start_pin, BasePin* end_pin);

	// Get error message from last validation
	static String METHOD(GetLastError)();

private:
	static Bool IsPassNode(BaseNode* node);
	static Bool IsResourceNode(BaseNode* node);
	static Bool IsValidPassToResource(BasePin* pass_pin, BasePin* res_pin);
	static Bool IsValidResourceToPass(BasePin* res_pin, BasePin* pass_pin);

#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	static String s_last_error;
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHCONNECTIONVALIDATOR_
