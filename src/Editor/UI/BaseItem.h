
#pragma once
#ifndef _BASEGLOBAL_
#define _BASEGLOBAL_

#include "Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class BaseItem;
class BaseLink;
class BaseNode;
class BasePin;

extern UInt64 g_editor_item_id ;
extern UInt64 GetNextEditorItemID();
extern Vector<BaseItem*> g_all_items;
extern BaseItem* GetItemByID(UInt64 id);

MYRENDERER_BEGIN_CLASS(BaseItem)
#pragma region METHOD
public:
	VIRTUAL ~BaseItem() MYDEFAULT;
	BaseItem() MYDEFAULT;
	BaseItem(CONST String& in_name, Bool in_show = true);
	BaseItem(CONST BaseItem& other) MYDELETE;
	BaseItem(BaseItem&& other) MYDELETE;
	BaseItem& operator=(CONST BaseItem& other) MYDELETE;
	BaseItem& operator=(BaseItem&& other) MYDELETE;

	VIRTUAL void METHOD(Draw)();
	VIRTUAL void METHOD(Release)();

	VIRTUAL BaseLink* METHOD(AsLink)() {return nullptr;}
	VIRTUAL BaseNode* METHOD(AsNode)() {return nullptr;}
	VIRTUAL BasePin* METHOD(AsPin)() { return nullptr; }

	UInt64 METHOD(GetSelfID)() CONST;
	CONST String& METHOD(GetName)();
	void METHOD(SetName)(CONST String& in_name);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";
	UInt64 self_id = 0;
private:
#pragma endregion
MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASEGLOBAL_