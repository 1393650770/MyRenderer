#include "EditorItemRegistry.h"
#include "BaseNode.h"
#include "BasePin.h"
#include "BaseLink.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

EditorItemRegistry* g_editor_registry = nullptr;

EditorItemRegistry::EditorItemRegistry()
{
}

EditorItemRegistry::~EditorItemRegistry()
{
}

NodeHandle EditorItemRegistry::RegisterNode(BaseNode* node)
{
	GenericHandle h = node_registry.Allocate(node, node->GetName());
	node->SetSelfHandle(h);
	return NodeHandle{ h };
}

PinHandle EditorItemRegistry::RegisterPin(BasePin* pin)
{
	GenericHandle h = pin_registry.Allocate(pin, pin->GetName());
	pin->SetSelfHandle(h);
	return PinHandle{ h };
}

LinkHandle EditorItemRegistry::RegisterLink(BaseLink* link)
{
	GenericHandle h = link_registry.Allocate(link, link->GetName());
	link->SetSelfHandle(h);
	return LinkHandle{ h };
}

BaseNode* EditorItemRegistry::ResolveNode(NodeHandle h) CONST
{
	return node_registry.Resolve(h.value);
}

BasePin* EditorItemRegistry::ResolvePin(PinHandle h) CONST
{
	return pin_registry.Resolve(h.value);
}

BaseLink* EditorItemRegistry::ResolveLink(LinkHandle h) CONST
{
	return link_registry.Resolve(h.value);
}

BaseItem* EditorItemRegistry::ResolveItem(GenericHandle handle) CONST
{
	//  --  --  --  --  --  --  --  --  --  --  --  --
	//  --  --  --  --   Node/Pin/Link  --  --  --  --  --  --
	BaseItem* item = STATIC_CAST(node_registry.Resolve(handle), BaseItem);
	if (item) return item;
	item = STATIC_CAST(pin_registry.Resolve(handle), BaseItem);
	if (item) return item;
	item = STATIC_CAST(link_registry.Resolve(handle), BaseItem);
	return item;
}

void EditorItemRegistry::RemoveNode(NodeHandle h)
{
	node_registry.Free(h.value);
}

void EditorItemRegistry::RemovePin(PinHandle h)
{
	pin_registry.Free(h.value);
}

void EditorItemRegistry::RemoveLink(LinkHandle h)
{
	link_registry.Free(h.value);
}

BaseItem* EditorItemRegistry::ResolveItemByIndex(UInt32 index) CONST
{
	BaseItem* item = node_registry.FindByIndex(index);
	if (item) return item;
	item = pin_registry.FindByIndex(index);
	if (item) return item;
	return link_registry.FindByIndex(index);
}

UInt32 EditorItemRegistry::GetNodeCount() CONST
{
	return node_registry.GetActiveCount();
}

UInt32 EditorItemRegistry::GetLinkCount() CONST
{
	return link_registry.GetActiveCount();
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
