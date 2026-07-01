#include "RenderGraphPanel.h"
#include "UI/BasePanel.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor_internal.h"
#include "UI/BasePin.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"
#include "RenderGraphPassNode.h"
#include "RenderGraphResourceNode.h"
#include "RenderGraphConnectionValidator.h"
#include "PropertiesPanel.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "RenderGraphSerializer.h"
#include "RenderGraphBuilder.h"
#include <iostream>
#include "Render/Core/RenderGraph.h"

namespace ed = ax::NodeEditor;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


RenderGraphPanel::RenderGraphPanel(CONST String& in_name, Bool in_show /*= true*/) : BasePanel(in_name, in_show)
{

}

void RenderGraphPanel::Init()
{
	ed::Config config;
	config.SettingsFile = nullptr;
	context = ed::CreateEditor(&config);
}

void RenderGraphPanel::Update()
{
}

void RenderGraphPanel::Draw()
{
	constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_NoScrollbar;
	if (OnBegin(windowFlags))
	{
		GraphMenu();

		// Full canvas for the node editor
		ImVec2 region = ImGui::GetContentRegionAvail();

		ed::SetCurrentEditor(context);
		ed::Begin("My Editor", region);
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

			// Handle node selection via click
			ed::NodeId selected_id;
			Int sel_count = ed::GetSelectedNodes(&selected_id, 1);
			if (sel_count > 0)
			{
				BaseNode* node = GetNode(selected_id.Get());
				if (node && node != selected_node)
				{
					selected_node = node;
				}
			}

			// Deselect on background double-click
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				ed::NodeId hovered = ed::GetHoveredNode();
				if (!hovered)
				{
					selected_node = nullptr;
				}
			}

			ed::End();
		}
		ed::SetCurrentEditor(nullptr);

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
			// Remove all links connected to this node's pins
			auto* node = *it;
			for (auto* pin : node->GetInputPins())
			{
				UInt64 pin_id = pin->GetSelfID();
				for (auto lit = links.begin(); lit != links.end(); )
				{
					BaseLink* link = *lit;
					if (link->GetStartID() == pin_id || link->GetEndID() == pin_id)
					{
						link->Release();
						delete link;
						lit = links.erase(lit);
					}
					else { ++lit; }
				}
			}
			for (auto* pin : node->GetOutputPins())
			{
				UInt64 pin_id = pin->GetSelfID();
				for (auto lit = links.begin(); lit != links.end(); )
				{
					BaseLink* link = *lit;
					if (link->GetStartID() == pin_id || link->GetEndID() == pin_id)
					{
						link->Release();
						delete link;
						lit = links.erase(lit);
					}
					else { ++lit; }
				}
			}

			if (selected_node == node)
	{
		selected_node = nullptr;
	}

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
	selected_node = nullptr;
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
		// Background context menu (right-click on empty space)
		if (ed::ShowBackgroundContextMenu())
		{
			ImGui::OpenPopup("CreateMenu");
		}

		// Node context menu (right-click on a node)
		if (ed::ShowNodeContextMenu(&node_id))
		{
			ImGui::OpenPopup("NodeContextMenu");
			hover_node_id = node_id.Get();
		}

		// Pin context menu
		if (ed::ShowPinContextMenu(&pin_id))
		{
			ImGui::OpenPopup("PinContextMenu");
			hover_pin_id = pin_id.Get();
			BasePin* pin = GetItemByID(hover_pin_id)->AsPin();
			hover_node_id = pin ? pin->GetBelongNode()->GetSelfID() : 0;
		}

		// Link context menu
		if (ed::ShowLinkContextMenu(&link_id))
		{
			ImGui::OpenPopup("LinkContextMenu");
			hover_link_id = link_id.Get();
		}
		ed::Resume();
	}

	// === Popups ===
	ed::Suspend();
	{
		// --- Create Menu (background right-click) ---
		if (ImGui::BeginPopup("CreateMenu"))
		{
			if (ImGui::BeginMenu("Add Pass"))
			{
				if (ImGui::MenuItem("Graphics Pass"))
				{
					auto* node = new RenderGraphPassNode("GraphicsPass", PassNodeType::Graphics);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("Compute Pass"))
				{
					auto* node = new RenderGraphPassNode("ComputePass", PassNodeType::Compute);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("Copy Pass"))
				{
					auto* node = new RenderGraphPassNode("CopyPass", PassNodeType::Copy);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("Custom Pass"))
				{
					auto* node = new RenderGraphPassNode("CustomPass", PassNodeType::Custom);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Add Resource"))
			{
				if (ImGui::MenuItem("RenderTarget (2D)"))
				{
					auto* node = new RenderGraphResourceNode("RenderTarget", ResourceNodeType::Texture);
					node->AddOutputPin("Output", PinAccess::Read);
					node->AddInputPin("Input", PinAccess::Write);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("DepthStencil"))
				{
					auto* node = new RenderGraphResourceNode("DepthStencil", ResourceNodeType::DepthStencil);
					node->AddOutputPin("Output", PinAccess::Read);
					node->AddInputPin("Input", PinAccess::Write);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("Buffer (Uniform)"))
				{
					auto* node = new RenderGraphResourceNode("UniformBuffer", ResourceNodeType::Buffer);
					node->SetBufferSize(256);
					node->SetBufferStride(16);
					node->AddOutputPin("Output", PinAccess::Read);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("Buffer (Storage)"))
				{
					auto* node = new RenderGraphResourceNode("StorageBuffer", ResourceNodeType::Buffer);
					node->SetBufferSize(4096);
					node->SetBufferStride(4);
					node->AddOutputPin("Output", PinAccess::Read);
					node->AddInputPin("Input", PinAccess::Write);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				if (ImGui::MenuItem("External Texture"))
				{
					auto* node = new RenderGraphResourceNode("ExternalRT", ResourceNodeType::ExternalTexture);
					node->SetIsTransient(false);
					node->AddOutputPin("Output", PinAccess::Read);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					nodes.push_back(node);
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		// --- Node Context Menu ---
		if (ImGui::BeginPopup("NodeContextMenu"))
		{
			BaseNode* node = GetNode(hover_node_id);

			// Rename
			if (node)
			{
				static Char name_buf[256];
				strncpy(name_buf, node->GetName().c_str(), sizeof(name_buf) - 1);
				ImGui::PushItemWidth(200);
				if (ImGui::InputText("##RenameNode", name_buf, sizeof(name_buf),
					ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					node->SetName(String(name_buf));
				}
				ImGui::PopItemWidth();
			}

			if (ImGui::MenuItem("Add Input Pin"))
			{
				if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
					pass->AddInputPin("Input", PinAccess::Read);
				else if (node)
					node->AddInput("Input");
			}
			if (ImGui::MenuItem("Add Output Pin"))
			{
				if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
					pass->AddOutputPin("Output", PinAccess::Write);
				else if (node)
					node->AddOutput("Output");
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete Node"))
			{
				if (node)
					DeleteItem(node->GetSelfID());
			}

			ImGui::EndPopup();
		}

		// --- Pin Context Menu ---
		if (ImGui::BeginPopup("PinContextMenu"))
		{
			BaseNode* node = GetNode(hover_node_id);
			BasePin* pin = node ? node->GetPin(hover_pin_id) : nullptr;
			if (pin)
			{
				static Char pin_name_buf[256];
				strncpy(pin_name_buf, pin->GetName().c_str(), sizeof(pin_name_buf) - 1);
				ImGui::PushItemWidth(200);
				if (ImGui::InputText("##RenamePin", pin_name_buf, sizeof(pin_name_buf),
					ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					pin->SetName(String(pin_name_buf));
					node->SetSetNeedRecalcSize();
				}
				ImGui::PopItemWidth();

				if (ImGui::MenuItem("Delete Pin"))
				{
					node->DeletePin(pin->GetSelfID());
				}
			}
			ImGui::EndPopup();
		}

		// --- Link Context Menu ---
		if (ImGui::BeginPopup("LinkContextMenu"))
		{
			if (ImGui::MenuItem("Delete Link"))
			{
				DeleteLink(hover_link_id);
			}
			ImGui::EndPopup();
		}

		ed::Resume();
	}
}

void RenderGraphPanel::BaseOperator()
{
	// === Link creation ===
	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
		ed::PinId input_pin_id, output_pin_id;
		if (ed::QueryNewLink(&input_pin_id, &output_pin_id))
		{
			if (input_pin_id && output_pin_id)
			{
				BasePin* start_pin = GetItemByID(output_pin_id.Get())->AsPin();
				BasePin* end_pin = GetItemByID(input_pin_id.Get())->AsPin();

				ConnectionResult result = RenderGraphConnectionValidator::Validate(start_pin, end_pin);

				if (result.is_valid)
				{
					if (ed::AcceptNewItem())
					{
						BaseLink* link = new BaseLink("Link");
						link->Init(output_pin_id.Get(), input_pin_id.Get());
						links.push_back(link);
						// Determine link color by dataflow direction
						PinAccess link_access = PinAccess::Read;
						BaseNode* start_node = start_pin->GetBelongNode();
						if (dynamic_cast<RenderGraphPassNode*>(start_node))
							link_access = start_pin->GetPinAccess(); // Pass output = Write or Create
						else
							link_access = PinAccess::Read; // Resource output = Read by pass
						link->SetLinkAccess(link_access);
					}
				}
				else
				{
					ed::RejectNewItem(ImColor(255, 80, 80), 2.0f);
					if (!result.error_message.empty())
					{
						ShowLabel(result.error_message.c_str());
					}
				}
			}
			else
			{
				ed::RejectNewItem(ImColor(255, 80, 80), 2.0f);
			}
		}
	}
	ed::EndCreate();

	// === Deletion ===
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

	// === Keyboard shortcuts ===
	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		Int selected_count = ed::GetSelectedObjectCount();
		Vector<ed::NodeId> selected_nodes(selected_count);
		Vector<ed::LinkId> selected_links(selected_count);
		ed::GetSelectedNodes(selected_nodes.data(), selected_count);
		ed::GetSelectedLinks(selected_links.data(), selected_count);
		for (auto& nid : selected_nodes)
			if (nid) DeleteNode(nid.Get());
		for (auto& lid : selected_links)
			if (lid) DeleteLink(lid.Get());
	}
}

void RenderGraphPanel::GraphMenu()
{
	static String current_save_path;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Graph", "Ctrl+N"))
			{
				while (!links.empty()) DeleteLink(links[0]->GetSelfID());
				while (!nodes.empty()) DeleteNode(nodes[0]->GetSelfID());
				current_save_path.clear();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Graph", "Ctrl+S"))
			{
				if (current_save_path.empty())
					current_save_path = "render_graph.rgraph.json";
				auto def = BuildDefinition();
				if (RenderGraphSerializer::SaveGraph(def, current_save_path))
					std::cout << "[RenderGraphEditor] Saved to: " << current_save_path << std::endl;
				else
					std::cerr << "[RenderGraphEditor] Save failed: " << RenderGraphSerializer::GetLastError() << std::endl;
			}
			if (ImGui::MenuItem("Save As..."))
			{
				auto def = BuildDefinition();
				String save_path = "render_graph_save.rgraph.json";
				if (RenderGraphSerializer::SaveGraph(def, save_path))
				{
					current_save_path = save_path;
					std::cout << "[RenderGraphEditor] Saved to: " << save_path << std::endl;
				}
				else
					std::cerr << "[RenderGraphEditor] Save failed: " << RenderGraphSerializer::GetLastError() << std::endl;
			}
			if (ImGui::MenuItem("Open...", "Ctrl+O"))
			{
				String load_path = current_save_path.empty() ? "render_graph.rgraph.json" : current_save_path;
				Render::RenderGraphDefinition def;
				if (RenderGraphSerializer::LoadGraph(def, load_path))
				{
					LoadDefinition(def);
					std::cout << "[RenderGraphEditor] Loaded from: " << load_path << std::endl;
				}
				else
					std::cerr << "[RenderGraphEditor] Load failed: " << RenderGraphSerializer::GetLastError() << std::endl;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				is_show = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Delete Selected", "Del"))
			{
				// handled in BaseOperator
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Navigate to Content"))
			{
				ed::NavigateToContent();
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
			return node;
	}
	return nullptr;
}

// ---- BuildDefinition / LoadDefinition ----

Render::RenderGraphDefinition RenderGraphPanel::BuildDefinition() CONST
{
	Render::RenderGraphDefinition def;
	def.graph_name = "RenderGraph";

	for (auto* node : nodes)
	{
		if (auto* pass_node = dynamic_cast<RenderGraphPassNode*>(node))
		{
			Render::RDGPassDef pd;
			pd.name = pass_node->GetName();
			pd.pass_kind = (Render::RDGPassKind)(Int)pass_node->GetPassType();
			for (auto* pin : pass_node->GetInputPins())
				pd.read_resources.push_back(pin->GetName());
			for (auto* pin : pass_node->GetOutputPins())
				pd.write_resources.push_back(pin->GetName());

			def.passes.push_back(pd);

			Render::RDGNodeLayout nl;
			nl.node_name = pass_node->GetName();
			def.node_layouts.push_back(nl);
		}
		else if (auto* res_node = dynamic_cast<RenderGraphResourceNode*>(node))
		{
			Render::RDGResourceDef rd;
			rd.name = res_node->GetName();
			rd.kind = (res_node->GetResourceType() == ResourceNodeType::Buffer)
				? Render::RDGResourceKind::Buffer : Render::RDGResourceKind::Texture;
			rd.texture_format = res_node->GetTextureFormat();
			rd.width = res_node->GetTextureWidth();
			rd.height = res_node->GetTextureHeight();
			rd.mip_level = res_node->GetMipLevel();
			rd.samples = res_node->GetSamples();
			rd.buffer_size = res_node->GetBufferSize();
			rd.buffer_stride = res_node->GetBufferStride();
			rd.is_transient = res_node->GetIsTransient();

			def.resources.push_back(rd);

			Render::RDGNodeLayout nl;
			nl.node_name = res_node->GetName();
			def.node_layouts.push_back(nl);
		}
	}
		// Save link edges
		for (auto* link : links)
		{
			BasePin* start_pin = GetItemByID(link->GetStartID()) ? GetItemByID(link->GetStartID())->AsPin() : nullptr;
			BasePin* end_pin = GetItemByID(link->GetEndID()) ? GetItemByID(link->GetEndID())->AsPin() : nullptr;
			if (!start_pin || !end_pin) continue;
			if (!start_pin->GetBelongNode() || !end_pin->GetBelongNode()) continue;

			Render::RDGEdgeDef edge;
			edge.source_node_name = start_pin->GetBelongNode()->GetName();
			edge.source_pin_name = start_pin->GetName();
			edge.target_node_name = end_pin->GetBelongNode()->GetName();
			edge.target_pin_name = end_pin->GetName();
			edge.edge_type = (Int)end_pin->GetPinAccess();
			def.edges.push_back(edge);
		}


	return def;
}

void RenderGraphPanel::LoadDefinition(CONST Render::RenderGraphDefinition& def)
{
	while (!links.empty()) DeleteLink(links[0]->GetSelfID());
	while (!nodes.empty()) DeleteNode(nodes[0]->GetSelfID());

	Map<String, BaseNode*> name_to_node;

	// Create resource nodes first
	for (auto& rd : def.resources)
	{
		ResourceNodeType rtype = ResourceNodeType::Texture;
		switch (rd.kind)
		{
		case Render::RDGResourceKind::Buffer:         rtype = ResourceNodeType::Buffer; break;
		case Render::RDGResourceKind::ExternalTexture: rtype = ResourceNodeType::ExternalTexture; break;
		case Render::RDGResourceKind::DepthStencil:    rtype = ResourceNodeType::DepthStencil; break;
		default:                                      rtype = ResourceNodeType::Texture; break;
		}
		auto* node = new RenderGraphResourceNode(rd.name, rtype);
		node->SetTextureFormat(rd.texture_format);
		node->SetTextureWidth(rd.width);
		node->SetTextureHeight(rd.height);
		node->SetMipLevel(rd.mip_level);
		node->SetSamples(rd.samples);
		node->SetBufferSize(rd.buffer_size);
		node->SetBufferStride(rd.buffer_stride);
		node->SetIsTransient(rd.is_transient);

		node->AddInputPin("Input", PinAccess::Write);
		node->AddOutputPin("Output", PinAccess::Read);

		nodes.push_back(node);
		name_to_node[rd.name] = node;
	}

	// Create pass nodes
	for (auto& pd : def.passes)
	{
		PassNodeType ptype = PassNodeType::Custom;
		switch (pd.pass_kind)
		{
		case Render::RDGPassKind::Graphics: ptype = PassNodeType::Graphics; break;
		case Render::RDGPassKind::Compute:  ptype = PassNodeType::Compute; break;
		case Render::RDGPassKind::Copy:     ptype = PassNodeType::Copy; break;
		default:                           ptype = PassNodeType::Custom; break;
		}
		auto* node = new RenderGraphPassNode(pd.name, ptype);
		for (auto& rname : pd.read_resources)
			node->AddInputPin(rname, PinAccess::Read);
		for (auto& rname : pd.write_resources)
			node->AddOutputPin(rname, PinAccess::Write);

		nodes.push_back(node);
		name_to_node[pd.name] = node;
	}

	// Restore positions
	for (auto& nl : def.node_layouts)
	{
		auto it = name_to_node.find(nl.node_name);
		if (it != name_to_node.end())
		{
		}
	// Restore links from edges
	for (auto& ed : def.edges)
		{
			auto src_it = name_to_node.find(ed.source_node_name);
			auto tgt_it = name_to_node.find(ed.target_node_name);
			if (src_it == name_to_node.end() || tgt_it == name_to_node.end()) continue;

			BasePin* src_pin = src_it->second->GetPinByName(ed.source_pin_name);
			BasePin* tgt_pin = tgt_it->second->GetPinByName(ed.target_pin_name);
			if (!src_pin || !tgt_pin) continue;

			BaseLink* link = new BaseLink("Link");
			link->Init(src_pin->GetSelfID(), tgt_pin->GetSelfID());
			link->SetLinkAccess((PinAccess)ed.edge_type);
			links.push_back(link);
		}
	}
}
void RenderGraphPanel::SyncRuntimeToEditor(Render::RenderGraph* graph)
{
	if (!graph) return;

	// Sync runtime passes
	for (auto& pass : graph->GetPasses())
	{
		String pass_name = pass->GetName();
		// Check if already exists
		bool exists = false;
		for (auto* n : nodes)
		{
			if (n->GetName() == pass_name)
			{ exists = true; break; }
		}
		if (exists) continue;

		auto* node = new RenderGraphPassNode(pass_name, PassNodeType::Custom);
		node->BindPass(pass.get());
		for (auto* res : pass->GetReadResources())
			node->AddInputPin(res->GetName(), PinAccess::Read);
		for (auto* res : pass->GetWriteResources())
			node->AddOutputPin(res->GetName(), PinAccess::Write);
		for (auto* res : pass->GetCreateResources())
			node->AddOutputPin(res->GetName(), PinAccess::Create);
		nodes.push_back(node);
	}

	// Sync runtime resources
	for (auto& res : graph->GetResources())
	{
		String res_name = res->GetName();
		// Check if already exists
		bool exists = false;
		for (auto* n : nodes)
		{
			if (n->GetName() == res_name)
			{ exists = true; break; }
		}
		if (exists) continue;

		ResourceNodeType rtype = ResourceNodeType::Texture;
		if (res->IsBufferResource()) rtype = ResourceNodeType::Buffer;
		auto* node = new RenderGraphResourceNode(res_name, rtype);
		node->SetIsTransient(res->GetIsTransient());
		node->BindResource(res.get());
		node->AddInputPin("Input", PinAccess::Write);
		node->AddOutputPin("Output", PinAccess::Read);
		nodes.push_back(node);
	}
}


PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
