#pragma once
#ifndef _BASEPIN_
#define _BASEPIN_

#include <imgui.h>

#include "Core/ConstDefine.h"
#include "BaseItem.h"
#include "EditorItemHandle.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

enum class PinType : UInt8
{
	Input,
	Output
};

// Forward-declare PinAccess from RenderGraphNodeColors
enum class PinAccess : UInt8;


MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BasePin,public BaseItem)
friend class BaseNode;
#pragma region METHOD
public:
	VIRTUAL ~BasePin() MYDEFAULT;
	BasePin() MYDEFAULT;
	BasePin(PinType in_pin_type, NodeHandle in_owner, PinAccess in_access = (PinAccess)0, CONST String& in_name="", Bool in_show = true);
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
	NodeHandle METHOD(GetOwnerHandle)() CONST { return owner_handle; }
	CONST PinType& METHOD(GetPinType)() CONST { return pin_type; }
	CONST PinAccess& METHOD(GetPinAccess)() CONST { return access_type; }
	void METHOD(SetPinAccess)(PinAccess in_access) { access_type = in_access; }
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	PinType pin_type = PinType::Input;
	PinAccess access_type = (PinAccess)0; // Read/Write/Create -- default Read
	NodeHandle owner_handle;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASEPIN_