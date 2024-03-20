#include "BaseNode.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "BasePin.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)



BaseNode::BaseNode( CONST String& in_name /*= ""*/, Bool in_show /*= true*/) : name(in_name), is_show(in_show)
{
	self_id = node_id++;
}

void BaseNode::AddInput(CONST String& in_name)
{
	input_pins.push_back(new BasePin(PinType::Input, in_name));
	is_need_resize = true;
}

void BaseNode::AddOutput(CONST String& in_name)
{
	output_pins.push_back(new BasePin(PinType::Output, in_name));
	is_need_resize = true;
}
UInt64 BaseNode::GetSelfID() CONST
{
	return self_id;
}

void BaseNode::RecalcSize()
{
	Float32 max_width = 0;
	Float32 max_height = 0;
	Int minpin_size = min(input_pins.size(), output_pins.size());
	for (Int i = 0; i < minpin_size; ++i)
	{
		ImVec2 input_size = input_pins[i]->GetSize();
		ImVec2 output_size = output_pins[i]->GetSize();
		max_width = max(max_width, input_size.x + output_size.x);
		max_height += max(input_size.y, output_size.y);
	}

	node_single_line_width = max_width;
	node_single_line_height = max_height;
}

void BaseNode::Init()
{

}

void BaseNode::Draw()
{
	ed::BeginNode(self_id);
	ImGui::Text(name.c_str());
	Int maxpin_size = max(input_pins.size(), output_pins.size());
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	Float32 lineWidth = node_single_line_width; // ���÷ָ����ĳ���
	draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + lineWidth, p.y), IM_COL32(255, 255, 255, 255), 2.0f);
	ImGui::Dummy(ImVec2(0, 5));
	for (Int i = 0; i < maxpin_size; ++i)
	{
		if (i < input_pins.size() && i < output_pins.size())
		{
			input_pins[i]->Draw();
			ImGui::SameLine(node_single_line_width - output_pins[i]->GetSize().x + 10);
			output_pins[i]->Draw();
		}
		else if (i < output_pins.size())
		{
			ImGui::NewLine();
			ImGui::SameLine(node_single_line_width - output_pins[i]->GetSize().x + 10);
			output_pins[i]->Draw();
		}
		else if (i < input_pins.size())
		{
			input_pins[i]->Draw();
		}
	}
	ed::EndNode();

	if (is_need_resize)
	{
		RecalcSize();
		is_need_resize = false;
	}
}

void BaseNode::Release()
{
	for (auto& pin : input_pins)
	{
		pin->Release();
		delete pin;
	}
	input_pins.clear();
	for (auto& pin : output_pins)
	{
		pin->Release();
		delete pin;
	}
}


UInt64 BaseNode::node_id=1;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE