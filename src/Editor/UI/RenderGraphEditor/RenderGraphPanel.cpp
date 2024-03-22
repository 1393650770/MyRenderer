#include "RenderGraphPanel.h"
#include "UI/BasePanel.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "UI/BasePin.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor_internal.h"

namespace ed = ax::NodeEditor;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


RenderGraphPanel::RenderGraphPanel(CONST String& in_name, Bool in_show /*= true*/) : BasePanel(in_name, in_show)
{

}

void RenderGraphPanel::Init()
{
	ed::Config config;
	context = ed::CreateEditor(&config); 
	//BaseNode* node = new BaseNode("NodeA");
	//node->AddInput("InputA");
	//node->AddInput("InputB");
	//node->AddInput("InputC");
	//node->AddOutput("OutputA");
	//node->AddOutput("OutputB");
	//node->AddOutput("OutputC");
	//nodes.push_back(node);
}

void RenderGraphPanel::Update()
{
}

void RenderGraphPanel::Draw()
{
	constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse| ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_NoScrollbar;
	if (OnBegin(windowFlags))
	{
		GraphMenu();
		ed::SetCurrentEditor(context);
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		{

			BaseOperator();
			CreateOperator();

			for (auto& node : nodes)
			{
				node->Draw();
			}
			for (auto& link : links)
			{
				link->Draw();
			}
			ed::End();
			ed::SetCurrentEditor(nullptr);
		}
		OnEnd();
	}
}

void RenderGraphPanel::DeleteLink(UInt64 id)
{
	for (auto it = links.begin(); it != links.end(); ++it)
	{
		if ((*it)->GetSelfID() == id)
		{
			(*it)->Release();
			delete (*it);
			links.erase(it);
			break;
		}
	}
}

void RenderGraphPanel::DeleteNode(UInt64 id)
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		if ((*it)->GetSelfID() == id)
		{
			(*it)->Release();
			delete (*it);
			nodes.erase(it);
			break;
		}
	}
}

void RenderGraphPanel::DeleteItem(UInt64 id)
{
	DeleteLink(id);
	DeleteNode(id);
}

void RenderGraphPanel::Release()
{
	for (auto& node : nodes)
	{
		node->Release();
		delete node;
	}
	nodes.clear();
	for (auto& link : links)
	{
		link->Release();
		delete link;
	}
	links.clear();
	ed::DestroyEditor(context);
	context = nullptr;
}

CONST String RenderGraphPanel::GetTypeName()
{
	return "RenderGraphPanel";
}

void RenderGraphPanel::CreateOperator()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ed::NodeId node_id = ed::GetHoveredNode();
	ed::PinId pin_id = ed::GetHoveredPin();
	ed::LinkId link_id = ed::GetHoveredLink();
	ed::Suspend();
	{
		if (ed::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("Create New Node");
		}
		if (ed::ShowNodeContextMenu(&node_id))
		{
			ImGui::OpenPopup("Create New Pin");
			hover_node_id = node_id.Get();
		}
		if (ed::ShowPinContextMenu(&pin_id))
		{
			ImGui::OpenPopup("Change PinName");
			hover_node_id = node_id.Get();
			hover_pin_id = pin_id.Get();
		}
		if (ed::ShowLinkContextMenu(&link_id))
		{
			ImGui::OpenPopup("Create New LinkName");
			hover_link_id = link_id.Get();
		}
		ed::Resume();
	}

	ed::Suspend();
	{
		if (ImGui::BeginPopup("Create New Node"))
		{
			if (ImGui::MenuItem("New Node"))
			{
				BaseNode* new_node= new BaseNode("New Node");
				ed::SetNodePosition(new_node->GetSelfID(), mouse_pos);
				nodes.push_back(new_node);
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("Create New Pin"))
		{
			BaseNode* node = GetNode(hover_node_id);
			if (node)
			{
				String new_name = node->GetName();
				ImGui::PushItemWidth( ImGui::CalcTextSize(new_name.c_str()).x + 20 );
				new_name.resize(256);
				if (ImGui::InputText("##ChangeNodeName", new_name.data(), 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					size_t pos = new_name.find_first_of('\0');
					String curlength_name = new_name.substr(0, pos);
					node->SetName(curlength_name);
				}
				ImGui::PopItemWidth();
			}
			if (ImGui::MenuItem("Add Input Pin"))
			{

				if (node)
				{
					node->AddInput("Input");
				}
			}
			if (ImGui::MenuItem("Add Output Pin"))
			{
				if (node)
				{
					node->AddOutput("Output");
				}
			}
			if (ImGui::MenuItem("Delete This Node"))
			{
				if (node)
				{
					DeleteItem(node->GetSelfID());
				}
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Change PinName"))
		{
			BaseNode* node = GetNode(hover_node_id);
			BasePin* pin = node ? node->GetPin(hover_pin_id) : nullptr;
			if (pin)
			{
				String new_name = pin->GetName();
				ImGui::PushItemWidth(ImGui::CalcTextSize(new_name.c_str()).x + 20);
				new_name.resize(256);
				if (ImGui::InputText("##ChangePinName", new_name.data(), 256, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					size_t pos = new_name.find_first_of('\0');
					String curlength_name = new_name.substr(0, pos);
					pin->SetName(curlength_name);
					node->SetSetNeedRecalcSize();
				}
				ImGui::PopItemWidth();
				if (ImGui::MenuItem("Delete This Pin"))
				{
					node->DeletePin(pin->GetSelfID());
				}

			}
			ImGui::EndPopup();
		}

		ed::Resume();
	}
}

void RenderGraphPanel::BaseOperator()
{
	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
		ed::PinId input_pin_id, output_pin_id;
		if (ed::QueryNewLink(&input_pin_id, &output_pin_id))
		{
			if (input_pin_id && output_pin_id)
			{
				BasePin* input_pin = GetItemByID(input_pin_id.Get())->AsPin();
				BasePin* output_pin = GetItemByID(output_pin_id.Get())->AsPin();
				if (ed::AcceptNewItem())
				{
					if (input_pin&&output_pin
						&& input_pin->GetBelongNode()!=output_pin->GetBelongNode()
						&& input_pin->GetPinType()!=output_pin->GetPinType())
					{
						links.push_back(new BaseLink("Link"));
						links.back()->Init(input_pin_id.Get(), output_pin_id.Get());
					}
					else
					{
						ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
					}
				}
				if (!(input_pin && output_pin))
				{
					ShowLabel("x Is Not Pin");
				}
				else if ((input_pin && output_pin) && (input_pin->GetBelongNode() == output_pin->GetBelongNode()))
				{
					ShowLabel("x Belong To Same Node");
				}
				else if ((input_pin && output_pin) && input_pin->GetPinType() == output_pin->GetPinType())
				{
					ShowLabel("x Same Pin Type");
				}

			}
			else
			{
				ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
			}

		}
	}
	ed::EndCreate();



	if (ed::BeginDelete())
	{
		ed::LinkId deleted_link_id;
		while (ed::QueryDeletedLink(&deleted_link_id))
		{

			if (ed::AcceptDeletedItem())
			{
				DeleteLink(deleted_link_id.Get());
			}
		}
		ed::NodeId deleted_node_id;
		while (ed::QueryDeletedNode(&deleted_node_id))
		{
			if (ed::AcceptDeletedItem())
			{
				DeleteNode(deleted_node_id.Get());
			}
		}
	}
	ed::EndDelete();
}

void RenderGraphPanel::GraphMenu()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("New Render Graph"))
			{

			}
			if (ImGui::MenuItem("Load Render Graph"))
			{

			}
			if (ImGui::MenuItem("Save Render Graph"))
			{

			}
			if (ImGui::BeginMenu("OtherSetting"))
			{

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit"))
			{
				is_show = false;
			}


			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Resource"))
		{
			if (ImGui::MenuItem("Add Resource"))
			{

			}

			ImGui::EndMenu();
		}


		ImGui::EndMenuBar();
	}
}

BaseNode* RenderGraphPanel::GetNode(UInt64 id)
{
	for (auto& node : nodes)
	{
		if (node->GetSelfID() == id)
		{
			return node;
		}
	}
	return nullptr;
}

PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE