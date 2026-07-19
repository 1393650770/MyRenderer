#include "BaseItem.h"
#include "EditorItemRegistry.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

//  --  --  --  --  --  --  --  --  --  --  --  --
static EditorItemRegistry* s_registry = nullptr;

EditorItemRegistry* GetEditorRegistry()
{
	if (s_registry == nullptr)
	{
		s_registry = new EditorItemRegistry();
		g_editor_registry = s_registry;
	}
	return s_registry;
}

BaseItem::BaseItem(CONST String& in_name, Bool in_show /*= true*/) : name(in_name), is_show(in_show)
{
	self_handle = kInvalidHandle;
}


void BaseItem::Draw()
{
}

void BaseItem::Release()
{
}

GenericHandle BaseItem::GetSelfHandle() CONST
{
	return self_handle;
}

void BaseItem::SetSelfHandle(GenericHandle h)
{
	self_handle = h;
}

CONST String& BaseItem::GetName()
{
	return name;
}

void BaseItem::SetName(CONST String& in_name)
{
	name = in_name;
}

//  --  --  --  --  --  --  --   handle  --  --  --   index  --  --  ID  --  --
UInt64 BaseItem::GetSelfID() CONST
{
	return GetHandleIndex(self_handle);
}

BaseItem* BaseItem::FindByIndex(UInt32 index)
{
	auto* reg = GetEditorRegistry();
	BaseItem* item = reg->ResolveItemByIndex(index);
	return item;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
