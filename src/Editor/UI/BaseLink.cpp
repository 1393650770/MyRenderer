#include "BaseLink.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

BaseLink::BaseLink(CONST String& in_name, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{

}

void BaseLink::Init(UInt64 start_id /*= 0*/, UInt64 end_id /*= 0*/)
{
	this->start_id = start_id;
	this->end_id = end_id;
}

void BaseLink::Draw()
{
	ImColor color = RenderGraphColors::GetLinkColor(link_access);
	ed::Link(self_id, start_id, end_id, color);
}

void BaseLink::Release()
{
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
