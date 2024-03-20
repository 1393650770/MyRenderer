#include "RenderGraphPanel.h"
#include "UI/BasePanel.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"

namespace ed = ax::NodeEditor;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


RenderGraphPanel::RenderGraphPanel(CONST String& in_name /*= "RenderGraphEditorPannel"*/, Bool in_show /*= true*/) : BasePanel(in_name, in_show)
{

}

void RenderGraphPanel::Init()
{
	ed::Config config;
	context = ed::CreateEditor(&config);

	BaseNode* node = new BaseNode("NodeA");
	node->AddInput("InputA");
	node->AddInput("InputB");
	node->AddInput("InputC");
	node->AddOutput("OutputA");
	node->AddOutput("OutputB");
	node->AddOutput("OutputC");
	nodes.push_back(node);
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
			ImGui::EndMenuBar();
		}
		//ImGuiItemStatusFlags_HoveredRect
		ed::SetCurrentEditor(context);
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		{
			/*
			int uniqueId = 1;
			int startPinId = 1, endPinId = 2;
			// Start drawing nodes.
			ed::BeginNode(uniqueId++);
			ImGui::Text("Node A");
			startPinId= uniqueId;
			ed::BeginPin(uniqueId++, ed::PinKind::Input);
			ImGui::Text("->");

			ed::EndPin();
			ImGui::SameLine();
			endPinId = uniqueId;
			ed::BeginPin(uniqueId++, ed::PinKind::Output);
			ImGui::Text("->");
			ed::EndPin();
			ed::EndNode();


			ed::BeginNode(uniqueId++);
			ed::BeginGroupHint(uniqueId++);
			ImGui::Text("GroupHint");
			endPinId = uniqueId;
			ed::BeginPin(uniqueId++, ed::PinKind::Output);
			ImGui::Text("->");
			ed::EndPin();
			ed::EndGroupHint();
			ed::EndNode();

			ed::Link(uniqueId++, startPinId, endPinId);

			ed::BeginNode(uniqueId++);
			ed::BeginShortcut();
			ImGui::Text("Shortcut");
			ed::EndShortcut();
			ed::EndNode();
			*/
			for (auto& node : nodes)
			{
				node->Draw();
			}
			for (auto& link : links)
			{
				link->Draw();
			}


			if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
			{
				ed::PinId inputPinId, outputPinId;
				if (ed::QueryNewLink(&inputPinId, &outputPinId))
				{
					if (inputPinId && outputPinId)
					{
						if (ed::AcceptNewItem())
						{
							links.push_back(new BaseLink("Link"));
							links.back()->Init(inputPinId.Get(), outputPinId.Get());
							links.back()->Draw();

							if (ed::GetLinkPins(links.back()->GetSelfID(), &inputPinId, &outputPinId))
							{

							}
						}

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
						for (auto it = links.begin(); it != links.end(); ++it)
						{
							if ((*it)->GetSelfID() == deleted_link_id.Get())
							{
								(*it)->Release();
								delete (*it);
								links.erase(it);
								break;
							}
						}
					}
				}
				ed::NodeId deleted_node_id;
				while (ed::QueryDeletedNode(&deleted_node_id))
				{
					if (ed::AcceptDeletedItem())
					{
						for (auto it = nodes.begin(); it != nodes.end(); ++it)
						{
							if ((*it)->GetSelfID() == deleted_node_id.Get())
							{
								(*it)->Release();
								delete (*it);
								nodes.erase(it);
								break;
							}
						}
					}
				}
			}
			ed::EndDelete();


			//if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			//{
			//	// 在这里执行右键被按下时的操作
			//}
			//else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			//{
			//	// 在这里执行左键被按下时的操作
			//	nodes[0]->AddOutput("OutputDsdasdadas");
			//}

			ed::End();
			ed::SetCurrentEditor(nullptr);
		}
		OnEnd();
	}
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

PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE