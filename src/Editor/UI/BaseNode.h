
#pragma once
#ifndef _BASENODE_
#define _BASENODE_

#include <imgui.h>

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class BasePin;

MYRENDERER_BEGIN_CLASS(BaseNode)
#pragma region METHOD
public:
	VIRTUAL ~BaseNode() MYDEFAULT;
	BaseNode() MYDEFAULT;
	BaseNode( CONST String& in_name = "", Bool in_show = true);
	BaseNode(CONST BaseNode& other) MYDELETE;
	BaseNode(BaseNode&& other) MYDELETE;
	BaseNode& operator=(CONST BaseNode& other) MYDELETE;
	BaseNode& operator=(BaseNode&& other) MYDELETE;

	VIRTUAL void METHOD(Init)();
	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();

	void METHOD(AddInput)(CONST String& in_name = "");
	void METHOD(AddOutput)(CONST String& in_name = "");

	UInt64 METHOD(GetSelfID)() CONST;
protected:
	VIRTUAL void METHOD(RecalcSize)();
private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";
	UInt64 self_id=0;
	static UInt64 node_id;
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