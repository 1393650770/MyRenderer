#include "BaseItem.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


UInt64 g_editor_item_id = 1;
Vector<BaseItem*> g_all_items;
UInt64 GetNextEditorItemID()
{
	return g_editor_item_id++;
}

BaseItem* GetItemByID(UInt64 id)
{
	for (auto& item : g_all_items)
	{
		if (item->GetSelfID() == id)
		{
			return item;
		}
	}
	return nullptr;
}


BaseItem::BaseItem(CONST String& in_name, Bool in_show /*= true*/) : name(in_name), is_show(in_show)
{
	self_id = GetNextEditorItemID();
	g_all_items.push_back(this);
}


void BaseItem::Draw()
{
}

void BaseItem::Release()
{
}

UInt64 BaseItem::GetSelfID() CONST
{
	return self_id;
}

void BaseItem::SetName(CONST String& in_name)
{
	name = in_name;
}

CONST String& BaseItem::GetName()
{
	return name;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE