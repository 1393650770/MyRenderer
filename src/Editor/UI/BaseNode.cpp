#include "BaseNode.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "BasePin.h"


namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


BaseNode::BaseNode(CONST String& in_name, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{
}

void BaseNode::AddInput(CONST String& in_name)
{
	input_pins.push_back(new BasePin(PinType::Input,this, in_name));
	is_need_resize = true;
}

void BaseNode::AddOutput(CONST String& in_name)
{
	output_pins.push_back(new BasePin(PinType::Output, this, in_name));
	is_need_resize = true;
}

void BaseNode::DeletePin(UInt64 id)
{
	for (Int i = 0; i < input_pins.size(); ++i)
	{
		if (input_pins[i]->GetSelfID() == id)
		{
			input_pins[i]->Release();
			delete input_pins[i];
			input_pins.erase(input_pins.begin() + i);
			is_need_resize = true;
			return;
		}
	}
	for (Int i = 0; i < output_pins.size(); ++i)
	{
		if (output_pins[i]->GetSelfID() == id)
		{
			output_pins[i]->Release();
			delete output_pins[i];
			output_pins.erase(output_pins.begin() + i);
			is_need_resize = true;
			return;
		}
	}
}

BasePin* BaseNode::GetPin(UInt64 id)
{
	for (auto& pin : input_pins)
	{
		if (pin->GetSelfID() == id)
		{
			return pin;
		}
	}
	for (auto& pin : output_pins)
	{
		if (pin->GetSelfID() == id)
		{
			return pin;
		}
	}
	return nullptr;

}


void BaseNode::SetSetNeedRecalcSize()
{
	is_need_resize = true;
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
	Float32 lineWidth = max( node_single_line_width+10 , ImGui::CalcTextSize(name.c_str()).x); // 设置分隔符的长度
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
			if (input_pins.size() > 0)
			{
				ImGui::NewLine();
				ImGui::SameLine(node_single_line_width - output_pins[i]->GetSize().x + 10);
			}
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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE