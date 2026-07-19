#pragma once
#ifndef _BASEGLOBAL_
#define _BASEGLOBAL_

#include "Core/ConstDefine.h"
#include "Core/ResourceHandle.h"
#include "EditorItemHandle.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class BaseItem;
class BaseLink;
class BaseNode;
class BasePin;
class EditorItemRegistry;

//  --  --  --  --  --  --  --  --  --  --  --  --
extern EditorItemRegistry* GetEditorRegistry();

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

	GenericHandle METHOD(GetSelfHandle)() CONST;
	void METHOD(SetSelfHandle)(GenericHandle h);
	CONST String& METHOD(GetName)();
	void METHOD(SetName)(CONST String& in_name);

	//  --  --  --  --  --  --  --  --  --  --
	UInt64 METHOD(GetSelfID)() CONST;

	// --  --  --  --  --  --  --  --（Panel  --  --  ax::NodeEditor  --  UInt64 ID）
	static BaseItem* METHOD(FindByIndex)(UInt32 index);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";
	GenericHandle self_handle = kInvalidHandle;
private:
#pragma endregion
MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASEGLOBAL_
