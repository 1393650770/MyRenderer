
#pragma once
#ifndef _BASELINK_
#define _BASELINK_

#include <imgui.h>

#include "Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

enum class PinType : UInt8
{
	Input,
	Output
};


MYRENDERER_BEGIN_CLASS(BasePin)
#pragma region METHOD
public:
	VIRTUAL ~BasePin() MYDEFAULT;
	BasePin() MYDEFAULT;
	BasePin(PinType in_pin_type = PinType::Input, CONST String& in_name="", Bool in_show = true);
	BasePin(CONST BasePin& other) MYDELETE;
	BasePin(BasePin&& other) MYDELETE;
	BasePin& operator=(CONST BasePin& other) MYDELETE;
	BasePin& operator=(BasePin&& other) MYDELETE;

	VIRTUAL void METHOD(Init)();
	VIRTUAL void METHOD(Draw)() ;
	VIRTUAL void METHOD(Release)();
	VIRTUAL ImVec2 METHOD(GetSize)();
	UInt64 METHOD(GetSelfID)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";
	UInt64 self_id = 0;
	PinType pin_type =PinType::Input;
	static UInt64 pin_id;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASENODE_