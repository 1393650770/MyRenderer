#include "RenderGraphResourceNode.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "UI/BasePin.h"

namespace ed = ax::NodeEditor;
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

RenderGraphResourceNode::RenderGraphResourceNode(CONST String& in_name, ResourceNodeType in_res_type, Bool in_show)
	: BaseNode(in_name, in_show)
	, resource_type(in_res_type)
{
}

BasePin* RenderGraphResourceNode::AddInputPin(CONST String& in_name, PinAccess access)
{
	BasePin* pin = new BasePin(PinType::Input, this, access, in_name);
	input_pins.push_back(pin);
	is_need_resize = true;
	return pin;
}

BasePin* RenderGraphResourceNode::AddOutputPin(CONST String& in_name, PinAccess access)
{
	BasePin* pin = new BasePin(PinType::Output, this, access, in_name);
	output_pins.push_back(pin);
	is_need_resize = true;
	return pin;
}

void RenderGraphResourceNode::BindResource(Render::RenderGraphResourceBase* res)
{
	bound_resource = res;
}

void RenderGraphResourceNode::Draw()
{
	if (has_pending_pos)
	{
		ed::SetNodePosition(self_id, ImVec2(pending_pos_x, pending_pos_y));
		has_pending_pos = false;
	}
	ed::BeginNode(self_id);

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImColor header_color = RenderGraphColors::GetResourceHeaderColor(resource_type);

	// Draw resource type icon + name
	String header_text;
	switch (resource_type)
	{
	case ResourceNodeType::Texture:
	case ResourceNodeType::ExternalTexture:
		header_text = "[RT] " + name;
		break;
	case ResourceNodeType::Buffer:
		header_text = "[Buf] " + name;
		break;
	case ResourceNodeType::DepthStencil:
		header_text = "[DS] " + name;
		break;
	default:
		header_text = "[?] " + name;
	}

	if (!is_transient)
		header_text = "[EXT] " + header_text;

	ImGui::TextColored(header_color, "%s", header_text.c_str());

	// Separator line
	ImVec2 p = ImGui::GetCursorScreenPos();
	Float32 line_width = max(node_single_line_width + 20.0f, ImGui::CalcTextSize(header_text.c_str()).x + 10.0f);
	draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + line_width, p.y),
		RenderGraphColors::GetSeparatorColor(), 2.0f);
	ImGui::Dummy(ImVec2(0, 5));

	// Draw resource info line
	ImColor info_color = RenderGraphColors::GetResourceBorderColor(resource_type, is_transient);
	if (resource_type == ResourceNodeType::Texture || resource_type == ResourceNodeType::ExternalTexture
		|| resource_type == ResourceNodeType::DepthStencil)
	{
		String info_line = std::to_string(texture_width) + "x" + std::to_string(texture_height);
		if (mip_level > 1) info_line += " Mip:" + std::to_string(mip_level);
		if (samples > 1) info_line += " MSAA:" + std::to_string(samples);
		ImGui::TextColored(info_color, "%s", info_line.c_str());
	}
	else
	{
		String info_line = std::to_string(buffer_size) + "B stride:" + std::to_string(buffer_stride);
		ImGui::TextColored(info_color, "%s", info_line.c_str());
	}

	// Separator before pins
	ImGui::Dummy(ImVec2(0, 2));
	ImVec2 p2 = ImGui::GetCursorScreenPos();
	draw_list->AddLine(ImVec2(p2.x, p2.y), ImVec2(p2.x + line_width, p2.y),
		ImColor(150, 150, 150, 150), 1.0f);
	ImGui::Dummy(ImVec2(0, 3));

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

void RenderGraphResourceNode::Release()
{
	BaseNode::Release();
	bound_resource = nullptr;
}

void RenderGraphResourceNode::RecalcSize()
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

	// Account for header and info line
	String header_text = "[RT] " + name;
	Float32 header_width = ImGui::CalcTextSize(header_text.c_str()).x + 20.0f;

	String info_line = std::to_string(texture_width) + "x" + std::to_string(texture_height);
	Float32 info_width = ImGui::CalcTextSize(info_line.c_str()).x + 20.0f;
	max_width = max(max(max_width, header_width), info_width);

	node_single_line_width = max_width;
	node_single_line_height = max_height;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
