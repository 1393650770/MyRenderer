
#pragma once
#ifndef _BASELINK_
#define _BASELINK_

#include <imgui.h>

#include "Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(BaseLink)
#pragma region METHOD
public:
	VIRTUAL ~BaseLink() MYDEFAULT;
	BaseLink() MYDEFAULT;
	BaseLink(CONST String& in_name="", Bool in_show = true);
	BaseLink(CONST BaseLink& other) MYDELETE;
	BaseLink(BaseLink&& other) MYDELETE;
	BaseLink& operator=(CONST BaseLink& other) MYDELETE;
	BaseLink& operator=(BaseLink&& other) MYDELETE;

	VIRTUAL void METHOD(Init)( UInt32 start_id = 0, UInt32 end_id = 0);
	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();

	UInt64 METHOD(GetSelfID)() CONST;
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";
	UInt64 self_id = 0,start_id = 0, end_id = 0;
	static UInt64 link_id;
private:
#pragma endregion

MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASENODE_