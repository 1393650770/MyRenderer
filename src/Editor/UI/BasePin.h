
#pragma once
#ifndef _BASEPIN_
#define _BASEPIN_

#include <imgui.h>

#include "Core/ConstDefine.h"
#include "BaseItem.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

enum class PinType : UInt8
{
	Input,
	Output
};


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BasePin,public BaseItem)
friend class BaseNode;
#pragma region METHOD
public:
	VIRTUAL ~BasePin() MYDEFAULT;
	BasePin() MYDEFAULT;
	BasePin(PinType in_pin_type,BaseNode* in_owner,  CONST String& in_name="", Bool in_show = true);
	BasePin(CONST BasePin& other) MYDELETE;
	BasePin(BasePin&& other) MYDELETE;
	BasePin& operator=(CONST BasePin& other) MYDELETE;
	BasePin& operator=(BasePin&& other) MYDELETE;

	VIRTUAL void METHOD(Init)();
	VIRTUAL void METHOD(Draw)() ;
	VIRTUAL void METHOD(Release)();
	VIRTUAL ImVec2 METHOD(GetSize)();
	VIRTUAL BasePin* METHOD(AsPin)() { return this; }
	BaseNode* METHOD(GetBelongNode)();
	CONST PinType& METHOD(GetPinType)() CONST { return pin_type; }
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	PinType pin_type =PinType::Input;
	BaseNode* owner = nullptr;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASEPIN_