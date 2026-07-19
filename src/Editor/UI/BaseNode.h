#pragma once
#ifndef _BASENODE_
#define _BASENODE_

#include "Core/ConstDefine.h"
#include "BaseItem.h"
#include "EditorItemHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class BasePin;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BaseNode,public BaseItem)
#pragma region METHOD
public:
	VIRTUAL ~BaseNode() MYDEFAULT;
	BaseNode() MYDEFAULT;
	BaseNode( CONST String& in_name, Bool in_show = true);
	BaseNode(CONST BaseNode& other) MYDELETE;
	BaseNode(BaseNode&& other) MYDELETE;
	BaseNode& operator=(CONST BaseNode& other) MYDELETE;
	BaseNode& operator=(BaseNode&& other) MYDELETE;

	VIRTUAL void METHOD(Init)();
	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();
	VIRTUAL BaseNode* METHOD(AsNode)() { return this; }
	void METHOD(AddInput)(CONST String& in_name = "");
	void METHOD(AddOutput)(CONST String& in_name = "");

	void METHOD(DeletePin)(PinHandle h);

	BasePin* METHOD(GetPin)(PinHandle h);
	BasePin* METHOD(GetPinByName)(CONST String& name);
	void METHOD(SetSetNeedRecalcSize)();

	// Pin access for property inspectors
	Vector<BasePin*>& METHOD(GetInputPins)() { return input_pins; }
	Vector<BasePin*>& METHOD(GetOutputPins)() { return output_pins; }

	// Deferred position (set before first Draw, applied inside Draw)
	Bool has_pending_pos = false;
	Float32 pending_pos_x = 0, pending_pos_y = 0;
	void METHOD(SetPendingPosition)(Float32 x, Float32 y)
	{
		has_pending_pos = true; pending_pos_x = x; pending_pos_y = y;
	}

protected:
	VIRTUAL void METHOD(RecalcSize)();
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	Vector<BasePin*> input_pins;
	Vector<BasePin*> output_pins;
	Float32 node_single_line_width = 0 , node_single_line_height = 0;
	Bool is_need_resize = false;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASENODE_