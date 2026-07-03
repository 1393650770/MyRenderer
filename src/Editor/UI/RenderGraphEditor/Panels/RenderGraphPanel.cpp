#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/BasePanel.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor.h"
#include "ThirdParty/imgui_node_editor/imgui_node_editor_internal.h"
#include "UI/BasePin.h"
#include "UI/BaseNode.h"
#include "UI/BaseLink.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"
#include "UI/RenderGraphEditor/Services/RenderGraphConnectionValidator.h"
#include "UI/RenderGraphEditor/Panels/PropertiesPanel.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "UI/RenderGraphEditor/Services/RenderGraphSerializer.h"
#include "UI/RenderGraphEditor/Services/RenderGraphBuilder.h"
#include <iostream>
#include "Render/Core/RenderGraph.h"
// -- [AI] Phase 1 services
#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"
#include "UI/RenderGraphEditor/Services/PassRegistry.h"
#include "UI/RenderGraphEditor/RenderGraphSubGraphNode.h"
#include <set>
#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"
#include "UI/RenderGraphEditor/Services/PassRegistry.h"
#include "UI/RenderGraphEditor/RenderGraphSubGraphNode.h"
#include <set>

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
				if (!show_resource_nodes && dynamic_cast<RenderGraphResourceNode*>(node))
				{
					// Draw minimal pin-only node for link connectivity
					ed::BeginNode(node->GetSelfID());
					ImGui::Dummy(ImVec2(4, 4)); // Tiny hitbox
					for (auto* pin : node->GetInputPins()) pin->Draw();
					for (auto* pin : node->GetOutputPins()) pin->Draw();
					ed::EndNode();
					continue;
				}
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
					EditorEventBus::Get().FireSelectionChanged(selected_node);
				}
			}

			// Deselect on background double-click
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				ed::NodeId hovered = ed::GetHoveredNode();
				if (!hovered)
				{
					selected_node = nullptr;
					EditorEventBus::Get().FireSelectionChanged(nullptr);
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
				// -- [AI] Categorized from PassRegistry
				for (auto& cat : PassRegistry::Get().GetCategories())
				{
					if (ImGui::BeginMenu(cat.c_str()))
					{
						for (auto& entry : PassRegistry::Get().GetByCategory(cat))
						{
							if (ImGui::MenuItem(entry.name.c_str()))
							{
								PassNodeType pt = PassNodeType::Custom;
								switch (entry.pass_kind) {
									case Render::RDGPassKind::Graphics: pt = PassNodeType::Graphics; break;
									case Render::RDGPassKind::Compute:  pt = PassNodeType::Compute; break;
									case Render::RDGPassKind::Copy:     pt = PassNodeType::Copy; break;
									default: break;
								}
								auto* node = new RenderGraphPassNode(entry.name, pt);
								ed::SetNodePosition(node->GetSelfID(), mouse_pos);
								nodes.push_back(node);
							}
						}
						ImGui::EndMenu();
					}
				}
				ImGui::Separator();
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

	
	// -- [AI] Copy/Paste
	static Render::RenderGraphDefinition s_clipboard;
	static Bool s_has_clipboard = false;
	if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift) {
		Int sc = ed::GetSelectedObjectCount();
		if (sc > 0) {
			Vector<ed::NodeId> sel(sc); ed::GetSelectedNodes(sel.data(), sc);
			s_clipboard = Render::RenderGraphDefinition(); s_has_clipboard = true;
			Set<String> snames;
			for (auto& nid : sel) { if (nid) { BaseNode* n = GetNode(nid.Get()); if (n) snames.insert(n->GetName()); } }
			for (auto& nid : sel) {
				if (!nid) continue; BaseNode* n = GetNode(nid.Get()); if (!n) continue;
				if (auto* pn = dynamic_cast<RenderGraphPassNode*>(n)) {
					Render::RDGPassDef pd; pd.name = pn->GetName();
					pd.pass_kind = (Render::RDGPassKind)(Int)pn->GetPassType();
					for (auto* p : pn->GetInputPins()) pd.read_resources.push_back(p->GetName());
					for (auto* p : pn->GetOutputPins()) pd.write_resources.push_back(p->GetName());
					s_clipboard.passes.push_back(pd);
				} else if (auto* rn = dynamic_cast<RenderGraphResourceNode*>(n)) {
					Render::RDGResourceDef rd; rd.name = rn->GetName();
					s_clipboard.resources.push_back(rd);
				}
			}
			for (auto* link : links) {
				if (!link) continue;
				BaseItem* si = GetItemByID(link->GetStartID()); BaseItem* ei = GetItemByID(link->GetEndID());
				if (!si || !ei) continue; BasePin* sp = si->AsPin(); BasePin* ep = ei->AsPin();
				if (!sp || !ep || !sp->GetBelongNode() || !ep->GetBelongNode()) continue;
				if (snames.count(sp->GetBelongNode()->GetName()) && snames.count(ep->GetBelongNode()->GetName())) {
					Render::RDGEdgeDef ed; ed.source_node_name=sp->GetBelongNode()->GetName();
					ed.source_pin_name=sp->GetName(); ed.target_node_name=ep->GetBelongNode()->GetName();
					ed.target_pin_name=ep->GetName(); ed.edge_type=(Int)ep->GetPinAccess();
					s_clipboard.edges.push_back(ed);
				}
			}
		}
	}
	if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::GetIO().KeyCtrl && s_has_clipboard) {
		static UInt32 pcnt = 0; Float32 ox=50+pcnt*20, oy=50+pcnt*20; pcnt++;
		Map<String,String> nmap;
		for (auto& rd : s_clipboard.resources) {
			String nn=rd.name+"_C"; nmap[rd.name]=nn;
			auto* node=new RenderGraphResourceNode(nn, ResourceNodeType::Texture);
			ed::SetNodePosition(node->GetSelfID(), ImVec2(300+ox,200+oy));
			nodes.push_back(node);
		}
		for (auto& pd : s_clipboard.passes) {
			String nn=pd.name+"_C"; nmap[pd.name]=nn;
			PassNodeType pt=PassNodeType::Custom;
			switch(pd.pass_kind){case Render::RDGPassKind::Graphics:pt=PassNodeType::Graphics;break;case Render::RDGPassKind::Compute:pt=PassNodeType::Compute;break;default:break;}
			auto* node=new RenderGraphPassNode(nn,pt);
			for (auto& rn:pd.read_resources) node->AddInputPin(rn,PinAccess::Read);
			for (auto& rn:pd.write_resources) node->AddOutputPin(rn,PinAccess::Write);
			ed::SetNodePosition(node->GetSelfID(),ImVec2(300+ox,200+oy));
			nodes.push_back(node);
		}
		for (auto& ed:s_clipboard.edges) {
			String sn=nmap.count(ed.source_node_name)?nmap[ed.source_node_name]:ed.source_node_name;
			String tn=nmap.count(ed.target_node_name)?nmap[ed.target_node_name]:ed.target_node_name;
			BaseNode *ns=nullptr,*nt=nullptr;
			for(auto* n:nodes){if(n->GetName()==sn)ns=n;if(n->GetName()==tn)nt=n;}
			if(!ns||!nt)continue;
			BasePin*sp=ns->GetPinByName(ed.source_pin_name);BasePin*tp=nt->GetPinByName(ed.target_pin_name);
			if(!sp||!tp)continue;
			BaseLink* l=new BaseLink("Link");l->Init(sp->GetSelfID(),tp->GetSelfID());l->SetLinkAccess((PinAccess)ed.edge_type);
			links.push_back(l);
		}
		EditorEventBus::Get().FireGraphModified();
	}
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
				auto vr = GraphValidator::Validate(def);
				if(!vr.is_valid) std::cerr << "[RG] Save with " << vr.errors.size() << " validation issues" << std::endl;
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
			ImGui::Separator();
			ImGui::MenuItem("Show Resource Nodes", nullptr, &show_resource_nodes);
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
			auto pass_pos = ed::GetNodePosition(pass_node->GetSelfID());
			nl.pos_x = pass_pos.x;
			nl.pos_y = pass_pos.y;
			def.node_layouts.push_back(nl);
		}
		else if (auto* res_node = dynamic_cast<RenderGraphResourceNode*>(node))
		{
			Render::RDGResourceDef rd;
			String _rn = res_node->GetName();
			size_t _at = _rn.find("@");
			if (_at != String::npos) _rn = _rn.substr(0, _at);
			rd.name = _rn;
			Bool _dup = false;
			for (auto& r : def.resources) { if (r.name == _rn) { _dup = true; break; } }
			if (_dup) continue;
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
			auto res_pos = ed::GetNodePosition(res_node->GetSelfID());
			nl.pos_x = res_pos.x;
			nl.pos_y = res_pos.y;
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
			it->second->SetPendingPosition(nl.pos_x, nl.pos_y);
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

	// --- Phase 1: collect new nodes ---
	struct NewPassInfo { RenderGraphPassNode* node; Int index; };
	struct NewResInfo  { RenderGraphResourceNode* node; String name; };
	Vector<NewPassInfo> new_passes;
	Vector<NewResInfo>  new_resources;

	for (auto& pass : graph->GetPasses())
	{
		String pass_name = pass->GetName();
		bool exists = false;
		for (auto* n : nodes) { if (n->GetName() == pass_name) { exists = true; break; } }
		if (exists) continue;

		auto* node = new RenderGraphPassNode(pass_name, PassNodeType::Custom);
		node->BindPass(pass.get());
		for (auto* res : pass->GetReadResources())  node->AddInputPin(res->GetName(), PinAccess::Read);
		for (auto* res : pass->GetWriteResources()) node->AddOutputPin(res->GetName(), PinAccess::Write);
		for (auto* res : pass->GetCreateResources())node->AddOutputPin(res->GetName(), PinAccess::Create);
		nodes.push_back(node);
		new_passes.push_back({node, (Int)new_passes.size()});
	}

	// -- [AI] Multi-copy: one display node per (resource,pass) pair
	for (auto& res : graph->GetResources())
	{
		String res_name = res->GetName();
		for (auto& pass : graph->GetPasses())
		{
			Bool used = false;
			for (auto* rp : pass->GetReadResources())  if (rp->GetName() == res_name) { used = true; break; }
			for (auto* wp : pass->GetWriteResources()) if (wp->GetName() == res_name) { used = true; break; }
			for (auto* cp : pass->GetCreateResources())if (cp->GetName() == res_name) { used = true; break; }
			if (!used) continue;
			String display_key = res_name + "@" + pass->GetName();
			bool exists = false;
			for (auto* n : nodes) { if (n->GetName() == display_key) { exists = true; break; } }
			if (exists) continue;
			ResourceNodeType rtype = res->IsBufferResource() ? ResourceNodeType::Buffer : ResourceNodeType::Texture;
			auto* node = new RenderGraphResourceNode(display_key, rtype);
			node->SetIsTransient(res->GetIsTransient());
			node->BindResource(res.get());
			node->AddInputPin("Input", PinAccess::Write);
			node->AddOutputPin("Output", PinAccess::Read);
			nodes.push_back(node);
			new_resources.push_back({node, res_name});
		}
	}

	// --- Phase 2: layout based on dependencies ---
	if (!new_passes.empty() || !new_resources.empty())
	{
		Int pass_count = (Int)graph->GetPasses().size();
		Int res_count  = (Int)graph->GetResources().size();
		const Float32 pass_x_start = 100.0f;
		const Float32 pass_x_step  = 280.0f;
		const Float32 pass_y       = 200.0f;
		Int res_lane = 0;

		// Place passes left to right
		for (auto& pi : new_passes)
			pi.node->SetPendingPosition(pass_x_start + pi.index * pass_x_step, pass_y);

		// -- [AI] Stagger resources above/below pass row
		for (auto& ri : new_resources)
		{
			Int writer_idx = -1;
			Int reader_idx = pass_count;
			for (Int pi = 0; pi < pass_count; ++pi)
			{
				auto& pass = graph->GetPasses()[pi];
				for (auto* w : pass->GetWriteResources())
					if (w->GetName() == ri.name) { writer_idx = pi; break; }
				for (auto* c : pass->GetCreateResources())
					if (c->GetName() == ri.name) { writer_idx = pi; break; }
				for (auto* r : pass->GetReadResources())
					if (r->GetName() == ri.name && reader_idx == pass_count) { reader_idx = pi; }
			}
			if (writer_idx < 0) writer_idx = 0;
			if (reader_idx >= pass_count) reader_idx = writer_idx + 1;

			Float32 mid_x = pass_x_start + ((writer_idx + reader_idx) * 0.5f) * pass_x_step;
			Float32 res_y = (res_lane++ % 2 == 0) ? 60.0f : 480.0f;
			ri.node->SetPendingPosition(mid_x, res_y);
		}
	}

	// Build name -> node map for link creation
	Map<String, BaseNode*> name_to_node;
	for (auto* n : nodes)
		name_to_node[n->GetName()] = n;

	// Create links: Pass <-> Resource
	for (auto& pass : graph->GetPasses())
	{
		BaseNode* pass_node = name_to_node[pass->GetName()];
		if (!pass_node) continue;

		// Read resources: Resource Output -> Pass Input
		for (auto* res : pass->GetReadResources())
		{
			String _k = res->GetName() + "@" + pass->GetName();
			BaseNode* res_node = name_to_node[_k];
			if (!res_node) res_node = name_to_node[res->GetName()];
			if (!res_node) continue;

			BasePin* res_out = res_node->GetPinByName("Output");
			BasePin* pass_in = nullptr;
			for (auto* p : pass_node->GetInputPins()) if (p->GetName() == res->GetName()) { pass_in = p; break; }
			if (!res_out || !pass_in) continue;

			// Check if link already exists
			bool link_exists = false;
			for (auto* l : links)
			{
				if (l->GetStartID() == res_out->GetSelfID() && l->GetEndID() == pass_in->GetSelfID())
				{ link_exists = true; break; }
			}
			if (link_exists) continue;

			BaseLink* link = new BaseLink("Link");
			link->Init(res_out->GetSelfID(), pass_in->GetSelfID());
			link->SetLinkAccess(PinAccess::Read);
			links.push_back(link);
		}

		// Write/Create resources: Pass Output -> Resource Input
		auto add_pass_to_res_links = [&](const Vector<const Render::RenderGraphResourceBase*>& res_list, PinAccess access)
		{
			for (auto* res : res_list)
			{
				String _wk = res->GetName() + "@" + pass->GetName();
				BaseNode* res_node = name_to_node[_wk];
				if (!res_node) res_node = name_to_node[res->GetName()];
				if (!res_node) continue;

				BasePin* pass_out = nullptr;
				for (auto* p : pass_node->GetOutputPins()) if (p->GetName() == res->GetName()) { pass_out = p; break; }
				BasePin* res_in = res_node->GetPinByName("Input");
				if (!pass_out || !res_in) continue;

				bool link_exists = false;
				for (auto* l : links)
				{
					if (l->GetStartID() == pass_out->GetSelfID() && l->GetEndID() == res_in->GetSelfID())
					{ link_exists = true; break; }
				}
				if (link_exists) continue;

				BaseLink* link = new BaseLink("Link");
				link->Init(pass_out->GetSelfID(), res_in->GetSelfID());
				link->SetLinkAccess(access);
				links.push_back(link);
			}
		};

		add_pass_to_res_links(pass->GetWriteResources(), PinAccess::Write);
		add_pass_to_res_links(pass->GetCreateResources(), PinAccess::Create);
	}
}


PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
