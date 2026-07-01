#include "UI/RenderGraphEditor/Panels/OutlinePanel.h"
#include "UI/BaseNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

OutlinePanel::OutlinePanel(CONST String& in_name, Bool in_show)
	: BasePanel(in_name, in_show)
{
}

void OutlinePanel::Init()
{
}

void OutlinePanel::Update()
{
}

void OutlinePanel::Draw()
{
	constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
	if (OnBegin(windowFlags))
	{
		if (!rg_panel)
		{
			ImGui::TextColored(ImColor(150, 150, 150, 200), "No graph loaded.");
	EditorEventBus::Get().TickFireGraphModified();
		OnEnd();
			return;
		}

		auto& nodes = rg_panel->GetNodes();

		// Count pass and resource nodes
		Int pass_count = 0, res_count = 0;
		for (auto* n : nodes)
		{
			if (dynamic_cast<RenderGraphPassNode*>(n)) pass_count++;
			else if (dynamic_cast<RenderGraphResourceNode*>(n)) res_count++;
		}

		// Passes section
		if (ImGui::CollapsingHeader("Passes", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(ImColor(150, 150, 150, 150), "(%d)", pass_count);
			ImGui::SameLine();
			if (ImGui::SmallButton("+##addpass"))
			{
				// Trigger add via rg_panel's context mechanism
			}

			for (auto* n : nodes)
			{
				if (auto* pass = dynamic_cast<RenderGraphPassNode*>(n))
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
					if (rg_panel->GetSelectedNode() == n)
						flags |= ImGuiTreeNodeFlags_Selected;

					ImGui::PushStyleColor(ImGuiCol_Text,
						(UInt32)RenderGraphColors::GetPassHeaderColor(pass->GetPassType()));
					Bool opened = ImGui::TreeNodeEx((void*)(UInt64)pass->GetSelfID(), flags, "%s", pass->GetName().c_str());
					ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
						rg_panel->SetSelectedNode(pass);

					if (opened) ImGui::TreePop();
				}
			}
		}

		// Resources section
		if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextColored(ImColor(150, 150, 150, 150), "(%d)", res_count);
			ImGui::SameLine();
			if (ImGui::SmallButton("+##addres"))
			{
				// Trigger add via rg_panel
			}

			for (auto* n : nodes)
			{
				if (auto* res = dynamic_cast<RenderGraphResourceNode*>(n))
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
					if (rg_panel->GetSelectedNode() == n)
						flags |= ImGuiTreeNodeFlags_Selected;

					ImGui::PushStyleColor(ImGuiCol_Text,
						(UInt32)RenderGraphColors::GetResourceHeaderColor(res->GetResourceType()));
					Bool opened = ImGui::TreeNodeEx((void*)(UInt64)res->GetSelfID(), flags, "%s", res->GetName().c_str());
					ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
						rg_panel->SetSelectedNode(res);

					if (opened) ImGui::TreePop();
				}
			}
		}

	EditorEventBus::Get().TickFireGraphModified();
		OnEnd();
	}
}

void OutlinePanel::Release()
{
	rg_panel = nullptr;
}

void OutlinePanel::SetDataSource(RenderGraphPanel* rgp)
{
	rg_panel = rgp;
}

CONST String OutlinePanel::GetTypeName()
{
	return "OutlinePanel";
}

PanelRegister RegisterOutlinePanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new OutlinePanel(in_name, in_show);
}, OutlinePanel::GetTypeName());

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
