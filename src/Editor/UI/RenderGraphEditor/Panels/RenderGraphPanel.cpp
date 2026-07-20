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
#include "Platform/FileDialog.h"
#include "Render/Core/RenderGraphSerializer.h"
#include "Render/Core/RenderGraphBuilder.h"
#include <iostream>
#include "Render/Core/RenderGraph.h"
// --  Phase 1 services
#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"
#include "Render/Core/PassRegistry.h"
#include "UI/RenderGraphEditor/RenderGraphSubGraphNode.h"
#include "UI/RenderGraphEditor/Commands/CreateNodeCmd.h"
#include "UI/RenderGraphEditor/Commands/DeleteNodeCmd.h"
#include "UI/RenderGraphEditor/Commands/CreateLinkCmd.h"
#include "UI/RenderGraphEditor/Commands/DeleteLinkCmd.h"
#include "UI/RenderGraphEditor/Commands/RenameCmd.h"
#include "UI/RenderGraphEditor/Commands/MoveNodeCmd.h"
#include "UI/RenderGraphEditor/Commands/NewGraphCmd.h"
#include "UI/RenderGraphEditor/Commands/OpenGraphCmd.h"
#include "UI/RenderGraphEditor/Commands/ModifyPinCmd.h"
#include "UI/RenderGraphEditor/Commands/EditorCommandQueue.h"
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
			if (!show_resource_nodes) {
				// Walk links: for each pass↔resource connection, record (resource_base → writer_pass_id / reader_pass_id)
				auto base_name = [](const String& s) { size_t a = s.find('@'); return (a == String::npos) ? s : s.substr(0, a); };
				Map<String, UInt64> res_writer, res_reader;
				for (auto* link : links) {
					if (!link) continue;
					BasePin* sp = BaseItem::FindByIndex((UInt32)link->GetStartID()) ? BaseItem::FindByIndex((UInt32)link->GetStartID())->AsPin() : nullptr;
					BasePin* ep = BaseItem::FindByIndex((UInt32)link->GetEndID())   ? BaseItem::FindByIndex((UInt32)link->GetEndID())->AsPin()   : nullptr;
					if (!sp || !ep) continue;
					BaseNode* sn = sp->GetBelongNode(), * en = ep->GetBelongNode();
					if (!sn || !en) continue;
					Bool sp_res = dynamic_cast<RenderGraphResourceNode*>(sn) != nullptr;
					Bool ep_res = dynamic_cast<RenderGraphResourceNode*>(en) != nullptr;
					if (!sp_res && !ep_res) continue; // skip pass→pass
					String base = sp_res ? base_name(sn->GetName()) : base_name(en->GetName());
					UInt64 pass_id = sp_res ? en->GetSelfID() : sn->GetSelfID();
					PinAccess acc = sp_res ? ep->GetPinAccess() : sp->GetPinAccess();
					if (acc == PinAccess::Write || acc == PinAccess::Create) res_writer[base] = pass_id;
					else res_reader[base] = pass_id;
				}
				// Draw bezier from writer to reader (if both exist), or stub from writer
				for (auto& kv : res_writer) {
					ImVec2 p1 = ed::GetNodePosition(ed::NodeId(kv.second));
					auto rit = res_reader.find(kv.first);
					if (rit != res_reader.end()) {
						ImVec2 p2 = ed::GetNodePosition(ed::NodeId(rit->second));
						ImDrawList* dl = ImGui::GetForegroundDrawList();
						dl->AddBezierCubic(p1, p1 + ImVec2(50, 0), p2 - ImVec2(50, 0), p2, IM_COL32(100, 160, 255, 180), 2.5f);
						ImVec2 mid((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f - 10.f);
						ImVec2 sz = ImGui::CalcTextSize(kv.first.c_str());
						dl->AddRectFilled(ImVec2(mid.x - sz.x * 0.5f - 3, mid.y - 1), ImVec2(mid.x + sz.x * 0.5f + 3, mid.y + sz.y + 1), IM_COL32(25, 25, 45, 230), 3.f);
						dl->AddText(ImVec2(mid.x - sz.x * 0.5f, mid.y), IM_COL32(180, 220, 255, 255), kv.first.c_str());
					} else {
						// Terminal write
						ImDrawList* dl = ImGui::GetForegroundDrawList();
						dl->AddLine(p1, ImVec2(p1.x + 40, p1.y), IM_COL32(160, 140, 80, 180), 2.0f);
						String label = kv.first + " (term)";
						ImVec2 sz = ImGui::CalcTextSize(label.c_str());
						dl->AddText(ImVec2(p1.x + 44, p1.y - sz.y * 0.5f), IM_COL32(160, 140, 80, 200), label.c_str());
					}
				}
			}
				// Always draw links (hidden resource nodes serve as pass->pass bridges)
				for (auto& link : links) { link->Draw(); }

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
			BasePin* pin = BaseItem::FindByIndex((UInt32)hover_pin_id)->AsPin();
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
				// --  Categorized from PassRegistry
				for (auto& cat : Render::PassRegistry::Get().GetCategories())
				{
					if (ImGui::BeginMenu(cat.c_str()))
					{
						for (auto& entry : Render::PassRegistry::Get().GetByCategory(cat))
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
								AddNodeWithCmd(node, mouse_pos);
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
					AddNodeWithCmd(node, mouse_pos);
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
					AddNodeWithCmd(node, mouse_pos);
				}
				if (ImGui::MenuItem("DepthStencil"))
				{
					auto* node = new RenderGraphResourceNode("DepthStencil", ResourceNodeType::DepthStencil);
					node->AddOutputPin("Output", PinAccess::Read);
					node->AddInputPin("Input", PinAccess::Write);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					AddNodeWithCmd(node, mouse_pos);
				}
				if (ImGui::MenuItem("Buffer (Uniform)"))
				{
					auto* node = new RenderGraphResourceNode("UniformBuffer", ResourceNodeType::Buffer);
					node->SetBufferSize(256);
					node->SetBufferStride(16);
					node->AddOutputPin("Output", PinAccess::Read);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					AddNodeWithCmd(node, mouse_pos);
				}
				if (ImGui::MenuItem("Buffer (Storage)"))
				{
					auto* node = new RenderGraphResourceNode("StorageBuffer", ResourceNodeType::Buffer);
					node->SetBufferSize(4096);
					node->SetBufferStride(4);
					node->AddOutputPin("Output", PinAccess::Read);
					node->AddInputPin("Input", PinAccess::Write);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					AddNodeWithCmd(node, mouse_pos);
				}
				if (ImGui::MenuItem("External Texture"))
				{
					auto* node = new RenderGraphResourceNode("ExternalRT", ResourceNodeType::ExternalTexture);
					node->SetIsTransient(false);
					node->AddOutputPin("Output", PinAccess::Read);
					ed::SetNodePosition(node->GetSelfID(), mouse_pos);
					AddNodeWithCmd(node, mouse_pos);
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
					String oldName = node->GetName();
					String newName(name_buf);
					UInt64 nid = hover_node_id;
					QUEUE_LAMBDA("Rename Node",
						{ auto* n = GetNode(nid); if (n) n->SetName(newName); },
						{ auto* n = GetNode(nid); if (n) n->SetName(oldName); }
					);
				}
				ImGui::PopItemWidth();
			}

			if (ImGui::MenuItem("Add Input Pin"))
			{
				if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
					QUEUE_CMD(ModifyPinCmd, NodeHandle{node->GetSelfHandle()}, ModifyPinCmd::EAction::AddInput);
				else if (node)
					QUEUE_CMD(ModifyPinCmd, NodeHandle{node->GetSelfHandle()}, ModifyPinCmd::EAction::AddInput);
			}
			if (ImGui::MenuItem("Add Output Pin"))
			{
				if (auto* pass = dynamic_cast<RenderGraphPassNode*>(node))
					QUEUE_CMD(ModifyPinCmd, NodeHandle{node->GetSelfHandle()}, ModifyPinCmd::EAction::AddOutput);
				else if (node)
					QUEUE_CMD(ModifyPinCmd, NodeHandle{node->GetSelfHandle()}, ModifyPinCmd::EAction::AddOutput);
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Delete Node"))
			{
				if (node)
					QUEUE_CMD(DeleteNodeCmd, NodeHandle{node->GetSelfHandle()});
			}

			ImGui::EndPopup();
		}

		// --- Pin Context Menu ---
		if (ImGui::BeginPopup("PinContextMenu"))
		{
			BaseNode* node = GetNode(hover_node_id);
			BasePin* pin = node ? node->GetPinByIndex((UInt32)hover_pin_id) : nullptr;
			if (pin)
			{
				static Char pin_name_buf[256];
				strncpy(pin_name_buf, pin->GetName().c_str(), sizeof(pin_name_buf) - 1);
				ImGui::PushItemWidth(200);
				if (ImGui::InputText("##RenamePin", pin_name_buf, sizeof(pin_name_buf),
					ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					String oldName = pin->GetName();
					String newName(pin_name_buf);
					UInt64 hid = hover_pin_id;
					UInt64 nid = hover_node_id;
					QUEUE_LAMBDA("Rename Pin",
						{ auto* n = GetNode(nid); if (n) { auto* p = n->GetPinByIndex((UInt32)hid); if (p) { p->SetName(newName); n->SetSetNeedRecalcSize(); } } },
						{ auto* n = GetNode(nid); if (n) { auto* p = n->GetPinByIndex((UInt32)hid); if (p) { p->SetName(oldName); n->SetSetNeedRecalcSize(); } } }
					);
				}
				ImGui::PopItemWidth();

				if (ImGui::MenuItem("Delete Pin"))
				{
					QUEUE_CMD(ModifyPinCmd, NodeHandle{node->GetSelfHandle()}, PinHandle{pin->GetSelfHandle()}, ModifyPinCmd::EAction::Delete);
				}
			}
			ImGui::EndPopup();
		}

		// --- Link Context Menu ---
		if (ImGui::BeginPopup("LinkContextMenu"))
		{
			if (ImGui::MenuItem("Delete Link"))
			{
				QUEUE_CMD(DeleteLinkCmd, LinkHandle::Make((UInt32)hover_link_id, 0));
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
				BasePin* start_pin = BaseItem::FindByIndex((UInt32)output_pin_id.Get())->AsPin();
				BasePin* end_pin = BaseItem::FindByIndex((UInt32)input_pin_id.Get())->AsPin();

				ConnectionResult result = RenderGraphConnectionValidator::Validate(start_pin, end_pin);

				if (result.is_valid)
				{
					if (ed::AcceptNewItem())
					{
						BaseLink* link = new BaseLink("Link");
						link->Init(PinHandle{start_pin->GetSelfHandle()}, PinHandle{end_pin->GetSelfHandle()});
						QUEUE_CMD(CreateLinkCmd, link);
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


	// --  Copy/Paste
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
				BaseItem* si = BaseItem::FindByIndex((UInt32)link->GetStartID()); BaseItem* ei = BaseItem::FindByIndex((UInt32)link->GetEndID());
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
		BEGIN_TXN("Paste Nodes");
		for (auto& rd : s_clipboard.resources) {
			String nn=rd.name+"_C"; nmap[rd.name]=nn;
			auto* node=new RenderGraphResourceNode(nn, ResourceNodeType::Texture);
			ed::SetNodePosition(node->GetSelfID(), ImVec2(300+ox,200+oy));
			QUEUE_CMD(CreateNodeCmd, node, ImVec2(300+ox,200+oy));
		}
		for (auto& pd : s_clipboard.passes) {
			String nn=pd.name+"_C"; nmap[pd.name]=nn;
			PassNodeType pt=PassNodeType::Custom;
			switch(pd.pass_kind){case Render::RDGPassKind::Graphics:pt=PassNodeType::Graphics;break;case Render::RDGPassKind::Compute:pt=PassNodeType::Compute;break;default:break;}
			auto* node=new RenderGraphPassNode(nn,pt);
			for (auto& rn:pd.read_resources) node->AddInputPin(rn,PinAccess::Read);
			for (auto& rn:pd.write_resources) node->AddOutputPin(rn,PinAccess::Write);
			ed::SetNodePosition(node->GetSelfID(),ImVec2(300+ox,200+oy));
			QUEUE_CMD(CreateNodeCmd, node, ImVec2(300+ox,200+oy));
		}
		for (auto& ed:s_clipboard.edges) {
			String sn=nmap.count(ed.source_node_name)?nmap[ed.source_node_name]:ed.source_node_name;
			String tn=nmap.count(ed.target_node_name)?nmap[ed.target_node_name]:ed.target_node_name;
			BaseNode *ns=nullptr,*nt=nullptr;
			for(auto* n:nodes){if(n->GetName()==sn)ns=n;if(n->GetName()==tn)nt=n;}
			if(!ns||!nt)continue;
			BasePin*sp=ns->GetPinByName(ed.source_pin_name);BasePin*tp=nt->GetPinByName(ed.target_pin_name);
			if(!sp||!tp)continue;
			BaseLink* l=new BaseLink("Link");l->Init(PinHandle{sp->GetSelfHandle()},PinHandle{tp->GetSelfHandle()});l->SetLinkAccess((PinAccess)ed.edge_type);
			QUEUE_CMD(CreateLinkCmd, l);
		}
		END_TXN();
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
				QUEUE_CMD(DeleteLinkCmd, LinkHandle::Make((UInt32)deleted_link_id.Get(), 0));
			}
		}
		ed::NodeId deleted_node_id;
		while (ed::QueryDeletedNode(&deleted_node_id))
		{
			if (ed::AcceptDeletedItem())
			{
				QUEUE_CMD(DeleteNodeCmd, NodeHandle::Make((UInt32)deleted_node_id.Get(), 0));
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
		BEGIN_TXN("Delete Selected");
		for (auto& nid : selected_nodes)
			if (nid) QUEUE_CMD(DeleteNodeCmd, NodeHandle::Make((UInt32)nid.Get(), 0));
		for (auto& lid : selected_links)
			if (lid) QUEUE_CMD(DeleteLinkCmd, LinkHandle::Make((UInt32)lid.Get(), 0));
		END_TXN();
	}
}

void RenderGraphPanel::AddNodeWithCmd(BaseNode* node, ImVec2 pos)
{
	cmd_queue.Enqueue(std::make_unique<CreateNodeCmd>(this, node, pos));
}

void RenderGraphPanel::AddLinkWithCmd(BaseLink* link)
{
	cmd_queue.Enqueue(std::make_unique<CreateLinkCmd>(this, link));
}

void RenderGraphPanel::DeleteNodeWithCmd(UInt64 node_id)
{
	cmd_queue.Enqueue(std::make_unique<DeleteNodeCmd>(this, NodeHandle::Make((UInt32)node_id, 0)));
}

void RenderGraphPanel::DeleteLinkWithCmd(UInt64 link_id)
{
	cmd_queue.Enqueue(std::make_unique<DeleteLinkCmd>(this, LinkHandle::Make((UInt32)link_id, 0)));
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
				QUEUE_CMD(NewGraphCmd);
				current_save_path.clear();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Graph", "Ctrl+S"))
			{
				if (current_save_path.empty())
				{
					current_save_path = MXRender::Platform::SaveFileDialog();
					if (current_save_path.empty()) return;
				}
				auto save_path = current_save_path;
				QUEUE_ACTION("Save Graph", {
					auto def = BuildDefinition();
					auto vr = Render::RenderGraphValidator::Validate(def);
					if(!vr.is_valid) std::cerr << "[RG] Save with " << vr.errors.size() << " validation issues" << std::endl;
					if (Render::RenderGraphSerializer::SaveGraph(def, save_path))
						std::cout << "[RenderGraphEditor] Saved to: " << save_path << std::endl;
					else
						std::cerr << "[RenderGraphEditor] Save failed: " << Render::RenderGraphSerializer::GetLastError() << std::endl;
				});
			}
			if (ImGui::MenuItem("Save As..."))
			{
				auto def = BuildDefinition();
				String save_path = MXRender::Platform::SaveFileDialog();
				if (!save_path.empty())
				{
					current_save_path = save_path;
					QUEUE_ACTION("Save As", {
						if (Render::RenderGraphSerializer::SaveGraph(def, save_path))
							std::cout << "[RenderGraphEditor] Saved to: " << save_path << std::endl;
						else
							std::cerr << "[RenderGraphEditor] Save failed: " << Render::RenderGraphSerializer::GetLastError() << std::endl;
					});
				}
			}
			if (ImGui::MenuItem("Open...", "Ctrl+O"))
			{
				String load_path = MXRender::Platform::OpenFileDialog();
				if (!load_path.empty()) {
					current_save_path = load_path;
					QUEUE_CMD(OpenGraphCmd, load_path);
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit"))
			{
				QUEUE_ACTION("Exit", { is_show = false; });
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			// Undo/Redo — execute IMMEDIATELY
			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, command_history.CanUndo()))
			{
				command_history.Undo();
				EditorEventBus::Get().FireGraphModified();
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, command_history.CanRedo()))
			{
				command_history.Redo();
				EditorEventBus::Get().FireGraphModified();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Delete Selected", "Del"))
			{
				// handled in BaseOperator
			}
			ImGui::EndMenu();
		}

		// Keyboard shortcuts for undo/redo
		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
		{
			if (command_history.CanUndo())
			{
				command_history.Undo();
				EditorEventBus::Get().FireGraphModified();
			}
		}
		if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y))
		{
			if (command_history.CanRedo())
			{
				command_history.Redo();
				EditorEventBus::Get().FireGraphModified();
			}
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Navigate to Content"))
			{
				QUEUE_ACTION("Navigate", { ed::NavigateToContent(); });
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

BaseNode* RenderGraphPanel::GetNodeByHandle(NodeHandle h)
{
	for (auto& node : nodes)
	{
		if (node->GetSelfHandle() == h.value)
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
			rd.is_transient = res_node->GetIsTransient();
			if (res_node->GetResourceType() == ResourceNodeType::Buffer)
			{
				RHI::BufferDesc bd;
				bd.size = res_node->GetBufferSize();
				bd.stride = res_node->GetBufferStride();
				rd.desc = bd;
			}
			else
			{
				RHI::TextureDesc td;
				td.format = static_cast<ENUM_TEXTURE_FORMAT>(res_node->GetTextureFormat());
				td.width = res_node->GetTextureWidth();
				td.height = res_node->GetTextureHeight();
				td.mip_level = res_node->GetMipLevel();
				td.samples = res_node->GetSamples();
				rd.desc = td;
			}

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
			BasePin* start_pin = BaseItem::FindByIndex((UInt32)link->GetStartID()) ? BaseItem::FindByIndex((UInt32)link->GetStartID())->AsPin() : nullptr;
			BasePin* end_pin = BaseItem::FindByIndex((UInt32)link->GetEndID()) ? BaseItem::FindByIndex((UInt32)link->GetEndID())->AsPin() : nullptr;
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
		ResourceNodeType rtype = std::holds_alternative<RHI::BufferDesc>(rd.desc)
			? ResourceNodeType::Buffer : ResourceNodeType::Texture;
		auto* node = new RenderGraphResourceNode(rd.name, rtype);
		if (std::holds_alternative<RHI::BufferDesc>(rd.desc))
		{
			auto& bd = std::get<RHI::BufferDesc>(rd.desc);
			node->SetBufferSize(bd.size);
			node->SetBufferStride(bd.stride);
		}
		else
		{
			auto& td = std::get<RHI::TextureDesc>(rd.desc);
			node->SetTextureFormat((Int)td.format);
			node->SetTextureWidth(td.width);
			node->SetTextureHeight(td.height);
			node->SetMipLevel(td.mip_level);
			node->SetSamples(td.samples);
		}
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
	}
	// --  Auto-resolve edges from pin names when no explicit edges defined
	if (def.edges.empty())
	{
		for (auto* node : nodes)
		{
			auto* pass_node = dynamic_cast<RenderGraphPassNode*>(node);
			if (!pass_node) continue;

			// Input pins (Read): Resource.Output -> Pass.Input
			for (auto* pin : pass_node->GetInputPins())
			{
				auto res_it = name_to_node.find(pin->GetName());
				if (res_it == name_to_node.end()) continue;
				BasePin* res_pin = res_it->second->GetPinByName("Output");
				if (!res_pin) continue;

				BaseLink* link = new BaseLink("Link");
				link->Init(PinHandle{res_pin->GetSelfHandle()}, PinHandle{pin->GetSelfHandle()});
				link->SetLinkAccess(PinAccess::Read);
				links.push_back(link);
			}

			// Output pins (Write): Pass.Output -> Resource.Input
			for (auto* pin : pass_node->GetOutputPins())
			{
				auto res_it = name_to_node.find(pin->GetName());
				if (res_it == name_to_node.end()) continue;
				BasePin* res_pin = res_it->second->GetPinByName("Input");
				if (!res_pin) continue;

				BaseLink* link = new BaseLink("Link");
				link->Init(PinHandle{pin->GetSelfHandle()}, PinHandle{res_pin->GetSelfHandle()});
				link->SetLinkAccess(PinAccess::Write);
				links.push_back(link);
			}
		}
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
			link->Init(PinHandle{src_pin->GetSelfHandle()}, PinHandle{tgt_pin->GetSelfHandle()});
			link->SetLinkAccess((PinAccess)ed.edge_type);
			links.push_back(link);
		}
}
// -- 
void RenderGraphPanel::RequestBuildDefinition(CONST Render::RenderGraphDefinition& def)
{
	pending_build_def = def;
	has_pending_build = true;
}

void RenderGraphPanel::SyncRuntimeToEditor(Render::RenderGraph* graph)
{
	if (!graph) return;

	Map<String, Set<String>> res_writers, res_readers;
	Map<String, Render::RenderGraphResourceBase*> res_runtime;
	Map<String, Int> pass_order;
	Int pi = 0;
	for (auto& pass : graph->GetPasses()) {
		auto& pn = pass->GetName();
		pass_order[pn] = pi++;
		for (auto* r : pass->GetReadResources()) {
			res_readers[r->GetName()].insert(pn);
			res_runtime[r->GetName()] = const_cast<Render::RenderGraphResourceBase*>(r);
		}
		for (auto* r : pass->GetWriteResources()) {
			res_writers[r->GetName()].insert(pn);
			res_runtime[r->GetName()] = const_cast<Render::RenderGraphResourceBase*>(r);
		}
		for (auto* r : pass->GetCreateResources()) {
			res_writers[r->GetName()].insert(pn);
			res_runtime[r->GetName()] = const_cast<Render::RenderGraphResourceBase*>(r);
		}
	}

	Map<String, BaseNode*> name_to_node;
	for (auto* n : nodes) name_to_node[n->GetName()] = n;
	for (auto& pass : graph->GetPasses()) {
		String pn = pass->GetName();
		if (name_to_node.count(pn)) continue;
		auto* node = new RenderGraphPassNode(pn, PassNodeType::Custom);
		node->BindPass(pass.get());
		for (auto* r : pass->GetReadResources())  node->AddInputPin(r->GetName(), PinAccess::Read);
		for (auto* r : pass->GetWriteResources()) node->AddOutputPin(r->GetName(), PinAccess::Write);
		for (auto* r : pass->GetCreateResources())node->AddOutputPin(r->GetName(), PinAccess::Create);
		nodes.push_back(node);
		name_to_node[pn] = node;
	}

	auto find_pin = [](Vector<BasePin*>& pins, const String& name) -> BasePin* {
		for (auto* p : pins) if (p->GetName() == name) return p;
		return nullptr;
	};

	Set<String> all_res;
	for (auto& kv : res_readers) all_res.insert(kv.first);
	for (auto& kv : res_writers) all_res.insert(kv.first);
	Set<String> used_writers;

	for (auto& rn : all_res) {
		ResourceNodeType rtype = res_runtime.count(rn) && res_runtime[rn]->IsBufferResource()
			? ResourceNodeType::Buffer : ResourceNodeType::Texture;
		Vector<String> writers, readers;
		if (res_writers.count(rn)) for (auto& w : res_writers[rn]) writers.push_back(w);
		if (res_readers.count(rn)) for (auto& r : res_readers[rn]) readers.push_back(r);
		std::sort(writers.begin(), writers.end(), [&](auto& a, auto& b) { return pass_order[a] < pass_order[b]; });
		std::sort(readers.begin(), readers.end(), [&](auto& a, auto& b) { return pass_order[a] < pass_order[b]; });

		for (auto& rd : readers) {
			String writer;
			for (Int i = (Int)writers.size() - 1; i >= 0; --i)
				if (pass_order[writers[i]] < pass_order[rd]) { writer = writers[i]; break; }
			if (writer.empty()) continue;
			String node_name = rn + "@" + writer + "@" + rd;
			if (name_to_node.count(node_name)) continue;
			used_writers.insert(rn + "|" + writer);

			auto* node = new RenderGraphResourceNode(node_name, rtype);
			if (res_runtime.count(rn)) { node->SetIsTransient(res_runtime[rn]->GetIsTransient()); node->BindResource(res_runtime[rn]); }
			node->AddInputPin("Input", PinAccess::Write);
			node->AddOutputPin("Output", PinAccess::Read);
			nodes.push_back(node);
			name_to_node[node_name] = node;
		}
	}

	for (auto& rn : all_res) {
		if (!res_writers.count(rn)) continue;
		ResourceNodeType rtype = res_runtime.count(rn) && res_runtime[rn]->IsBufferResource()
			? ResourceNodeType::Buffer : ResourceNodeType::Texture;
		for (auto& w : res_writers[rn]) {
			if (used_writers.count(rn + "|" + w)) continue;
			String node_name = rn + "@" + w;
			if (name_to_node.count(node_name)) continue;
			auto* node = new RenderGraphResourceNode(node_name, rtype);
			if (res_runtime.count(rn)) { node->SetIsTransient(res_runtime[rn]->GetIsTransient()); node->BindResource(res_runtime[rn]); }
			node->AddInputPin("Input", PinAccess::Write);
			nodes.push_back(node);
			name_to_node[node_name] = node;
		}
	}

	auto make_link = [&](BasePin* src, BasePin* dst, PinAccess access) {
		if (!src || !dst) return;
		bool dup = false;
		for (auto* l : links) if (l->GetStartID() == src->GetSelfID() && l->GetEndID() == dst->GetSelfID()) { dup = true; break; }
		if (dup) return;
		auto* l = new BaseLink("Link"); l->Init(PinHandle{src->GetSelfHandle()}, PinHandle{dst->GetSelfHandle()}); l->SetLinkAccess(access);
		links.push_back(l);
	};

	for (auto& rn : all_res) {
		if (!res_readers.count(rn) || !res_writers.count(rn)) continue;
		Vector<String> writers, readers;
		for (auto& w : res_writers[rn]) writers.push_back(w);
		for (auto& r : res_readers[rn]) readers.push_back(r);
		std::sort(writers.begin(), writers.end(), [&](auto& a, auto& b) { return pass_order[a] < pass_order[b]; });
		std::sort(readers.begin(), readers.end(), [&](auto& a, auto& b) { return pass_order[a] < pass_order[b]; });

		for (auto& rd : readers) {
			String writer;
			for (Int i = (Int)writers.size() - 1; i >= 0; --i)
				if (pass_order[writers[i]] < pass_order[rd]) { writer = writers[i]; break; }
			if (writer.empty()) continue;
			String node_name = rn + "@" + writer + "@" + rd;
			BaseNode* res_node = name_to_node[node_name];
			if (!res_node || !name_to_node.count(writer) || !name_to_node.count(rd)) continue;
			make_link(find_pin(name_to_node[writer]->GetOutputPins(), rn),
				find_pin(res_node->GetInputPins(), "Input"), PinAccess::Write);
			make_link(find_pin(res_node->GetOutputPins(), "Output"),
				find_pin(name_to_node[rd]->GetInputPins(), rn), PinAccess::Read);
		}
	}
	for (auto& rn : all_res) {
		if (!res_writers.count(rn)) continue;
		for (auto& w : res_writers[rn]) {
			if (used_writers.count(rn + "|" + w)) continue;
			BaseNode* res_node = name_to_node[rn + "@" + w];
			if (!res_node || !name_to_node.count(w)) continue;
			make_link(find_pin(name_to_node[w]->GetOutputPins(), rn),
				find_pin(res_node->GetInputPins(), "Input"), PinAccess::Write);
		}
	}

	const float px = 100.0f, sx = 280.0f, py = 250.0f, ry0 = 50.0f, ry1 = 450.0f;
	for (auto* n : nodes) {
		auto* pass_n = dynamic_cast<RenderGraphPassNode*>(n);
		if (pass_n && pass_order.count(pass_n->GetName()) && !pass_n->has_pending_pos)
			pass_n->SetPendingPosition(px + pass_order[pass_n->GetName()] * sx, py);
	}
	Int lane = 0;
	for (auto& rn : all_res) {
		float ry = (lane++ % 2 == 0) ? ry0 : ry1;
		for (auto* n : nodes) {
			auto* rgn = dynamic_cast<RenderGraphResourceNode*>(n);
			if (!rgn || rgn->has_pending_pos) continue;
			String nm = rgn->GetName();
			if (nm.find(rn + "@") != 0) continue;
			String rest = nm.substr(rn.size() + 1);
			size_t at2 = rest.find('@');
			if (at2 == String::npos) {
				if (pass_order.count(rest))
					rgn->SetPendingPosition(px + pass_order[rest] * sx, ry);
			} else {
				String w = rest.substr(0, at2);
				String r = rest.substr(at2 + 1);
				if (pass_order.count(w) && pass_order.count(r))
					rgn->SetPendingPosition(px + (pass_order[w] + pass_order[r]) * 0.5f * sx, ry);
			}
		}
	}
}
PanelRegister RegisterRenderGraphPanel([](CONST String& in_name, Bool in_show) -> BasePanel*
{
	return new RenderGraphPanel(in_name, in_show);
}, RenderGraphPanel::GetTypeName());


MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
