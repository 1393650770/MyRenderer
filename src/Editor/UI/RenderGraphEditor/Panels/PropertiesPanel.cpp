#include "UI/RenderGraphEditor/Panels/PropertiesPanel.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// Simple format name list (corresponds to TextureFomat enum in RenderGraph.fbs)
static CONST Char* s_texture_format_names[] = {
	"None", "BC1", "BC1A", "BC2", "BC3", "BC4", "BC5", "BC6H", "BC7",
	"ETC1", "ETC2", "ETC2A", "ETC2A1", "PTC12", "PTC14", "PTC12A", "PTC14A", "PTC22", "PTC24",
	"ATC", "ATCE", "ATCI", "ASTC4x4", "ASTC5x5", "ASTC6x6", "ASTC8x5", "ASTC8x6", "ASTC10x5",
	"Unknown",
	"R1", "A8", "R8", "R8I", "R8U", "R8S", "R16", "R16I", "R16U", "R16F", "R16S",
	"R32I", "R32U", "R32F", "RG8", "RG8I", "RG8U", "RG8S", "RG16", "RG16I", "RG16U", "RG16F", "RG16S",
	"RG32I", "RG32U", "RG32F", "RGB8", "RGB8I", "RGB8U", "RGB8S", "RGB9E5F",
	"RGB16I", "RGB16U", "RGB16F", "RGB32I", "RGB32U", "RGB32F",
	"BGRA8", "RGBA8", "RGBA8I", "RGBA8U", "RGBA8S", "RGBA16", "RGBA16I", "RGBA16U", "RGBA16F", "RGBA16S",
	"RGBA32I", "RGBA32U", "RGBA32F", "R5G6B5", "RGBA4", "RGB5A1", "RGB10A2", "RG11B10F",
	"UnknownDepth",
	"D16", "D24", "D24S8", "D32", "D32FS8", "D16F", "D24F", "D32F", "D0S8"
};
static CONST Int s_texture_format_count = sizeof(s_texture_format_names) / sizeof(s_texture_format_names[0]);

PropertiesPanel::PropertiesPanel(CONST String& in_name, Bool in_show)
	: BasePanel(in_name, in_show)
{
}

void PropertiesPanel::Init()
{
}

void PropertiesPanel::Update()
{
}

void PropertiesPanel::Draw()
{
	constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
	if (OnBegin(windowFlags))
	{
		// Sync selection from RenderGraphPanel
		if (rg_panel && rg_panel->GetSelectedNode() != selected_node)
			selected_node = rg_panel->GetSelectedNode();

		if (!selected_node)
		{
			DrawEmptySelection();
		}
		else if (auto* pass_node = selected_node->AsNode())
		{
			// Check if it's actually a RenderGraphPassNode
			if (auto* rg_pass = dynamic_cast<RenderGraphPassNode*>(pass_node))
			{
				DrawPassProperties(rg_pass);
			}
			else if (auto* rg_res = dynamic_cast<RenderGraphResourceNode*>(pass_node))
			{
				DrawResourceProperties(rg_res);
			}
			else
			{
				ImGui::Text("Node: %s", selected_node->GetName().c_str());
				ImGui::Separator();
				ImGui::Text("Generic node — no editable properties.");
			}
		}
		// --   Poll EventBus for graph changes
	EditorEventBus::Get().TickFireGraphModified();
		OnEnd();
	}
}

void PropertiesPanel::DrawEmptySelection()
{
	ImGui::TextColored(ImColor(150, 150, 150, 200), "No node selected.");
	ImGui::Separator();
	ImGui::TextWrapped("Select a Pass or Resource node in the graph to edit its properties.");
}

void PropertiesPanel::DrawPassProperties(RenderGraphPassNode* pass_node)
{
	ImGui::TextColored(RenderGraphColors::GetPassHeaderColor(pass_node->GetPassType()),
		"Pass: %s", pass_node->GetName().c_str());
	ImGui::Separator();

	// Name editor
	static Char name_buf[256];
	strncpy(name_buf, pass_node->GetName().c_str(), sizeof(name_buf) - 1);
	name_buf[sizeof(name_buf) - 1] = '\0';
	if (ImGui::InputText("Name", name_buf, sizeof(name_buf)))
	{
		pass_node->SetName(String(name_buf));
	}

	// Pass type combo
	Int current_type = (Int)pass_node->GetPassType();
	CONST Char* type_names[] = { "Graphics", "Compute", "Copy", "UI", "Custom" };
	if (ImGui::Combo("Type", &current_type, type_names, 5))
	{
		pass_node->SetPassType((PassNodeType)current_type);
	}

	// Execution order (read-only or editable if not bound)
	ImGui::LabelText("Exec Order", "%d", pass_node->GetExecutionOrder());

	// Pin lists
	if (ImGui::CollapsingHeader("Input Pins (Read)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* pin : pass_node->GetInputPins())
		{
			ImGui::BulletText("%s [%s]", pin->GetName().c_str(),
				RenderGraphColors::GetPinAccessName(pin->GetPinAccess()));
			ImGui::SameLine();
			if (ImGui::SmallButton((String("X##delin_") + std::to_string(pin->GetSelfID())).c_str()))
			{
				pass_node->DeletePin(PinHandle{pin->GetSelfHandle()});
				break;
			}
		}
		if (ImGui::Button("+ Add Input"))
		{
			pass_node->AddInputPin("Input", PinAccess::Read);
		}
	}

	if (ImGui::CollapsingHeader("Output Pins (Write/Create)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* pin : pass_node->GetOutputPins())
		{
			ImGui::BulletText("%s [%s]", pin->GetName().c_str(),
				RenderGraphColors::GetPinAccessName(pin->GetPinAccess()));
			ImGui::SameLine();
			if (ImGui::SmallButton((String("X##delout_") + std::to_string(pin->GetSelfID())).c_str()))
			{
				pass_node->DeletePin(PinHandle{pin->GetSelfHandle()});
				break;
			}
		}
		if (ImGui::Button("+ Add Output"))
		{
			pass_node->AddOutputPin("Output", PinAccess::Write);
		}
	}
}

void PropertiesPanel::DrawResourceProperties(RenderGraphResourceNode* res_node)
{
	ImGui::TextColored(
		RenderGraphColors::GetResourceHeaderColor(res_node->GetResourceType()),
		"Resource: %s", res_node->GetName().c_str());
	ImGui::Separator();

	// Name editor
	static Char name_buf[256];
	strncpy(name_buf, res_node->GetName().c_str(), sizeof(name_buf) - 1);
	name_buf[sizeof(name_buf) - 1] = '\0';
	if (ImGui::InputText("Name", name_buf, sizeof(name_buf)))
	{
		res_node->SetName(String(name_buf));
	}

	// Resource type combo
	Int current_type = (Int)res_node->GetResourceType();
	CONST Char* res_type_names[] = { "Texture", "Buffer", "ExternalTexture", "DepthStencil" };
	if (ImGui::Combo("Type", &current_type, res_type_names, 4))
	{
		res_node->SetResourceType((ResourceNodeType)current_type);
	}

	// Transient flag
	Bool is_transient = res_node->GetIsTransient();
	if (ImGui::Checkbox("Transient", &is_transient))
	{
		res_node->SetIsTransient(is_transient);
	}
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Transient resources are created and destroyed within the graph.\nExternal resources are imported from outside.");

	// --- Texture properties ---
	if (res_node->GetResourceType() == ResourceNodeType::Texture ||
		res_node->GetResourceType() == ResourceNodeType::ExternalTexture ||
		res_node->GetResourceType() == ResourceNodeType::DepthStencil)
	{
		if (ImGui::CollapsingHeader("Texture Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Int w = (Int)res_node->GetTextureWidth();
			if (ImGui::InputInt("Width", &w)) res_node->SetTextureWidth((UInt32)max(1, w));

			Int h = (Int)res_node->GetTextureHeight();
			if (ImGui::InputInt("Height", &h)) res_node->SetTextureHeight((UInt32)max(1, h));

			Int mip = (Int)res_node->GetMipLevel();
			if (ImGui::InputInt("Mip Levels", &mip)) res_node->SetMipLevel((UInt8)max(1, mip));

			Int samples = (Int)res_node->GetSamples();
			if (ImGui::InputInt("Samples", &samples)) res_node->SetSamples((UInt8)max(1, samples));

			// Format combo
			Int current_fmt = res_node->GetTextureFormat();
			if (ImGui::Combo("Format", &current_fmt, s_texture_format_names, s_texture_format_count))
			{
				res_node->SetTextureFormat(current_fmt);
			}
		}
	}
	// --- Buffer properties ---
	else
	{
		if (ImGui::CollapsingHeader("Buffer Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Int size = (Int)res_node->GetBufferSize();
			if (ImGui::InputInt("Size (bytes)", &size)) res_node->SetBufferSize((UInt64)max(1, size));

			Int stride = (Int)res_node->GetBufferStride();
			if (ImGui::InputInt("Stride", &stride)) res_node->SetBufferStride((UInt32)max(1, stride));
		}
	}

	// Lifetime info (read-only)
	if (ImGui::CollapsingHeader("Lifetime Info"))
	{
		ImGui::LabelText("Realize Step", "%d", res_node->GetRealizeStep());
		ImGui::LabelText("Derealize Step", "%d", res_node->GetDerealizeStep());
	}

	// Pin lists
	if (ImGui::CollapsingHeader("Input Pins (Write source)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* pin : res_node->GetInputPins())
		{
			ImGui::BulletText("%s [%s]", pin->GetName().c_str(),
				RenderGraphColors::GetPinAccessName(pin->GetPinAccess()));
		}
	}

	if (ImGui::CollapsingHeader("Output Pins (Read targets)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (auto* pin : res_node->GetOutputPins())
		{
			ImGui::BulletText("%s [%s]", pin->GetName().c_str(),
				RenderGraphColors::GetPinAccessName(pin->GetPinAccess()));
		}
	}
}

void PropertiesPanel::Release()
{
	selected_node = nullptr;
}

void PropertiesPanel::SetSelectedNode(BaseNode* node)
{
	selected_node = node;
}

CONST String PropertiesPanel::GetTypeName()
{
	return "PropertiesPanel";
}

PanelRegister RegisterPropertiesPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new PropertiesPanel(in_name, in_show);
}, PropertiesPanel::GetTypeName());

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
