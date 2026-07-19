#pragma once
#ifndef _EDITORITEMREGISTRY_
#define _EDITORITEMREGISTRY_

#include "Core/ResourceRegistry.h"
#include "EditorItemHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BasePin;
class BaseLink;
class BaseItem;

// --  Editor  --  --  --  --  --  --  --  --  --
//  --  --   g_all_items + g_editor_item_id + GetItemByID  --  --  --
//  --  --   generation  --  --  --  --   O(1)  --  --  --
MYRENDERER_BEGIN_CLASS(EditorItemRegistry)
#pragma region METHOD
public:
	EditorItemRegistry();
	~EditorItemRegistry();

	// --  --  --  --  handle  --  --  --
	NodeHandle METHOD(RegisterNode)(BaseNode* node);
	PinHandle  METHOD(RegisterPin)(BasePin* pin);
	LinkHandle METHOD(RegisterLink)(BaseLink* link);

	// --  --  --  --  --  --   stale handle  --  --  nullptr  --  --
	BaseNode* METHOD(ResolveNode)(NodeHandle h) CONST;
	BasePin*  METHOD(ResolvePin)(PinHandle h) CONST;
	BaseLink* METHOD(ResolveLink)(LinkHandle h) CONST;

	//  --  --  --  --  handle  --  --  --   ID  --  --   BaseItem  --  --
	BaseItem* METHOD(ResolveItem)(GenericHandle handle) CONST;
	//  --  --  --  index  --  --（Panel  --  --  ax::NodeEditor  --  UInt64  --  --）
	BaseItem* METHOD(ResolveItemByIndex)(UInt32 index) CONST;

	// --  --  --  --  --  -- bump generation --  --
	void METHOD(RemoveNode)(NodeHandle h);
	void METHOD(RemovePin)(PinHandle h);
	void METHOD(RemoveLink)(LinkHandle h);

	// --  --  --  --  --  --
	UInt32 METHOD(GetNodeCount)() CONST;
	UInt32 METHOD(GetLinkCount)() CONST;
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
	ResourceRegistry<BaseNode> node_registry;
	ResourceRegistry<BasePin>  pin_registry;
	ResourceRegistry<BaseLink> link_registry;
private:
#pragma endregion
MYRENDERER_END_CLASS

//  --  --  --  --  --  --  --  --  --
extern EditorItemRegistry* g_editor_registry;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
