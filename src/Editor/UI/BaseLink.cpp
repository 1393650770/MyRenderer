#include "BaseLink.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

BaseLink::BaseLink( CONST String& in_name/*=""*/, Bool in_show /*= true*/) : name(in_name), is_show(in_show)
{
	self_id = link_id++;
}

void BaseLink::Init(UInt32 start_id /*= 0*/, UInt32 end_id /*= 0*/)
{
	this->start_id = start_id;
	this->end_id = end_id;
}

void BaseLink::Draw()
{
	ed::Link(self_id, start_id, end_id);
}

void BaseLink::Release()
{
}
UInt64 BaseLink::GetSelfID() CONST
{
	return self_id;
}

UInt64 BaseLink::link_id=1;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE