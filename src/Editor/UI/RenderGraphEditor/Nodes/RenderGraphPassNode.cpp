#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "UI/BasePin.h"
#include "UI/EditorItemRegistry.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

RenderGraphPassNode::RenderGraphPassNode(CONST String& in_name, PassNodeType in_pass_type, Bool in_show)
	: BaseNode(in_name, in_show)
	, pass_type(in_pass_type)
{
}

BasePin* RenderGraphPassNode::AddInputPin(CONST String& in_name, PinAccess access)
{
	BasePin* pin = new BasePin(PinType::Input, NodeHandle{ self_handle }, access, in_name);
	GetEditorRegistry()->RegisterPin(pin);
	input_pins.push_back(pin);
	is_need_resize = true;
	return pin;
}

BasePin* RenderGraphPassNode::AddOutputPin(CONST String& in_name, PinAccess access)
{
	BasePin* pin = new BasePin(PinType::Output, NodeHandle{ self_handle }, access, in_name);
	GetEditorRegistry()->RegisterPin(pin);
	output_pins.push_back(pin);
	is_need_resize = true;
	return pin;
}

void RenderGraphPassNode::BindPass(Render::RenderGraphPassBase* pass)
{
	bound_pass = pass;
}

void RenderGraphPassNode::Draw()
{
	if (has_pending_pos)
	{
		ed::SetNodePosition(GetHandleIndex(self_handle), ImVec2(pending_pos_x, pending_pos_y));
		has_pending_pos = false;
	}
	ed::BeginNode(GetHandleIndex(self_handle));

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 node_pos = ImGui::GetCursorScreenPos();
	ImColor header_color = RenderGraphColors::GetPassHeaderColor(pass_type);

	if (is_culled)
	{
		header_color = RenderGraphColors::GetCulledColor();
	}
	if (is_active)
	{
		header_color = RenderGraphColors::GetActiveColor();
	}

	// Header background
	Float32 header_height = ImGui::GetTextLineHeight() + 8.0f;
	Float32 node_width = max(node_single_line_width + 20.0f, ImGui::CalcTextSize(name.c_str()).x + 20.0f);

	// Draw pass type badge and name in header
	String header_text = String("[") + RenderGraphColors::GetPassTypeName(pass_type) + "] " + name;
	if (is_culled)
		header_text = "[CULLED] " + header_text;
	if (execution_order >= 0)
		header_text += " #" + std::to_string(execution_order);

	ImGui::TextColored(header_color, "%s", header_text.c_str());

	// Separator line
	ImVec2 p = ImGui::GetCursorScreenPos();
	Float32 line_width = max(node_single_line_width + 20.0f, ImGui::CalcTextSize(header_text.c_str()).x);
	draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + line_width, p.y),
		RenderGraphColors::GetSeparatorColor(), 2.0f);
	ImGui::Dummy(ImVec2(0, 5));

	// Draw pins (same layout as BaseNode)
	Int maxpin_size = max(input_pins.size(), output_pins.size());
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

void RenderGraphPassNode::Release()
{
	BaseNode::Release();
	bound_pass = nullptr;
}

void RenderGraphPassNode::RecalcSize()
{
	// Include header text width in size calculation
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

	// Account for header
	String header_text = String("[") + RenderGraphColors::GetPassTypeName(pass_type) + "] " + name;
	Float32 header_width = ImGui::CalcTextSize(header_text.c_str()).x;
	max_width = max(max_width, header_width);

	node_single_line_width = max_width;
	node_single_line_height = max_height;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
