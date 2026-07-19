#include "BaseLink.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

BaseLink::BaseLink(CONST String& in_name, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{

}

void BaseLink::Init(PinHandle start_pin /*= {}*/, PinHandle end_pin /*= {}*/)
{
	this->start_handle = start_pin;
	this->end_handle = end_pin;
}

void BaseLink::Draw()
{
	ImColor color = RenderGraphColors::GetLinkColor(link_access);
	ed::Link(GetHandleIndex(self_handle), start_handle.GetIndex(), end_handle.GetIndex(), color);
}

void BaseLink::Release()
{
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
