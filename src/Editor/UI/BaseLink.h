#pragma once
#ifndef _BASELINK_
#define _BASELINK_

#include "Core/ConstDefine.h"
#include "BaseItem.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BaseLink,public BaseItem)
#pragma region METHOD
public:
	VIRTUAL ~BaseLink() MYDEFAULT;
	BaseLink() MYDEFAULT;
	BaseLink(CONST String& in_name, Bool in_show = true);
	BaseLink(CONST BaseLink& other) MYDELETE;
	BaseLink(BaseLink&& other) MYDELETE;
	BaseLink& operator=(CONST BaseLink& other) MYDELETE;
	BaseLink& operator=(BaseLink&& other) MYDELETE;

	VIRTUAL void METHOD(Init)( PinHandle start_pin = {}, PinHandle end_pin = {});
	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();
	VIRTUAL BaseLink* METHOD(AsLink)() { return this; }
	PinHandle METHOD(GetStartHandle)() CONST { return start_handle; }
	PinHandle METHOD(GetEndHandle)() CONST { return end_handle; }

	//  --  --  --  --  --  --   ID  --  --  --
	UInt64 METHOD(GetStartID)() CONST { return start_handle.GetIndex(); }
	UInt64 METHOD(GetEndID)()   CONST { return end_handle.GetIndex(); }

	// Link color by access type
	void METHOD(SetLinkAccess)(PinAccess a) { link_access = a; }
	PinAccess METHOD(GetLinkAccess)() CONST { return link_access; }
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	PinHandle start_handle, end_handle;
	PinAccess link_access = PinAccess::Read;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASENODE_
