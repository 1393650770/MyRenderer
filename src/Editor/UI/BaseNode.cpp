#include "BaseNode.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "BasePin.h"
#include "EditorItemRegistry.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"


namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


BaseNode::BaseNode(CONST String& in_name, Bool in_show /*= true*/) : BaseItem(in_name, in_show)
{
}

void BaseNode::AddInput(CONST String& in_name)
{
	auto* pin = new BasePin(PinType::Input, NodeHandle{ self_handle }, PinAccess::Read, in_name);
	GetEditorRegistry()->RegisterPin(pin);
	input_pins.push_back(pin);
	is_need_resize = true;
}

void BaseNode::AddOutput(CONST String& in_name)
{
	auto* pin = new BasePin(PinType::Output, NodeHandle{ self_handle }, PinAccess::Write, in_name);
	GetEditorRegistry()->RegisterPin(pin);
	output_pins.push_back(pin);
	is_need_resize = true;
}

void BaseNode::DeletePin(PinHandle h)
{
	for (Int i = 0; i < input_pins.size(); ++i)
	{
		if (input_pins[i]->GetSelfHandle() == h.value)
		{
			input_pins[i]->Release();
			GetEditorRegistry()->RemovePin(h);
			delete input_pins[i];
			input_pins.erase(input_pins.begin() + i);
			is_need_resize = true;
			return;
		}
	}
	for (Int i = 0; i < output_pins.size(); ++i)
	{
		if (output_pins[i]->GetSelfHandle() == h.value)
		{
			output_pins[i]->Release();
			GetEditorRegistry()->RemovePin(h);
			delete output_pins[i];
			output_pins.erase(output_pins.begin() + i);
			is_need_resize = true;
			return;
		}
	}
}

BasePin* BaseNode::GetPin(PinHandle h)
{
	for (auto& pin : input_pins)
	{
		if (pin->GetSelfHandle() == h.value)
		{
			return pin;
		}
	}
	for (auto& pin : output_pins)
	{
		if (pin->GetSelfHandle() == h.value)
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
	// Apply deferred position (set by SyncRuntimeToEditor before first draw)
	if (has_pending_pos)
	{
		ed::SetNodePosition(GetHandleIndex(self_handle), ImVec2(pending_pos_x, pending_pos_y));
		has_pending_pos = false;
	}

	ed::BeginNode(GetHandleIndex(self_handle));
	ImGui::Text(name.c_str());
	Int maxpin_size = max(input_pins.size(), output_pins.size());
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	Float32 lineWidth = max( node_single_line_width+10 , ImGui::CalcTextSize(name.c_str()).x);
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
	auto* reg = GetEditorRegistry();
	for (auto& pin : input_pins)
	{
		PinHandle ph{ pin->GetSelfHandle() };
		pin->Release();
		reg->RemovePin(ph);
		delete pin;
	}
	input_pins.clear();
	for (auto& pin : output_pins)
	{
		PinHandle ph{ pin->GetSelfHandle() };
		pin->Release();
		reg->RemovePin(ph);
		delete pin;
	}
}
BasePin* BaseNode::GetPinByName(CONST String& name)
{
	for (auto* p : input_pins) if (p->GetName() == name) return p;
	for (auto* p : output_pins) if (p->GetName() == name) return p;
	return nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
