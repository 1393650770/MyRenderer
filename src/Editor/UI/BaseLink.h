
#pragma once
#ifndef _BASELINK_
#define _BASELINK_

#include "Core/ConstDefine.h"
#include "BaseItem.h"


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

	VIRTUAL void METHOD(Init)( UInt32 start_id = 0, UInt32 end_id = 0);
	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();
	VIRTUAL BaseLink* METHOD(AsLink)() { return this; }
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
		UInt64 start_id = 0, end_id = 0;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASENODE_