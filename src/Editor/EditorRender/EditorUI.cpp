#include "EditorUI.h"
#include "Application/Window.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderTexture.h"
#include "RHI/Vulkan/VK_CommandBuffer.h"
#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include "UI/BasePanel.h"
#include "UI/RenderGraphEditor/Panels/RenderGraphPanel.h"
#include "UI/RenderGraphEditor/Panels/PropertiesPanel.h"
#include "UI/RenderGraphEditor/Panels/OutlinePanel.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

using namespace Render;
using namespace RHI;
using namespace UI;
void EditorUI::Init(PlatformWindow* in_window, RHI::Viewport* in_viewport)
{
	m_window = in_window;
	IMGUI_CHECKVERSION();
	ImGuiContext* context = ImGui::CreateContext();
	CHECK_WITH_LOG(context ==nullptr,"Failed to create ImGui context!");
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	io.Fonts->AddFontDefault();
	io.Fonts->Build();
	//ImFont* font1 = io.Fonts->AddFontDefault();

	GLFWwindow* glfw_window = (GLFWwindow*)in_window->GetNativeHandle();

	CHECK_WITH_LOG(ImGui_ImplGlfw_InitForVulkan(glfw_window, true) == false, "Failed to init ImGui for Vulkan!");
	in_viewport->AttachUiLayer(this);

	// Register all panels (DockSpace will auto-arrange them)
	AddPanelUI(RenderGraphPanel::GetTypeName());
	AddPanelUI(PropertiesPanel::GetTypeName());
	AddPanelUI(OutlinePanel::GetTypeName());

	// Cache the RenderGraphPanel reference and wire up data sources
	for (auto* p : panels)
	{
		if (auto* rg = dynamic_cast<UI::RenderGraphPanel*>(p))
		{
			rg_panel = rg;
		}
	}
	// Wire OutlinePanel to access RenderGraphPanel's node list
	if (rg_panel)
	{
		for (auto* p : panels)
		{
			if (auto* outline = dynamic_cast<UI::OutlinePanel*>(p))
				outline->SetDataSource(rg_panel);
			if (auto* props = dynamic_cast<UI::PropertiesPanel*>(p))
				props->SetDataSource(rg_panel);
		}
	}
}

//  Logic thread: ImGui NewFrame + widgets + Render → returns ImDrawData
ImDrawData* EditorUI::DrawFrame_Logic()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (show_editor)
	{
		ImGuiViewport* main_vp = ImGui::GetMainViewport();
		ImVec2 editor_pos = ImVec2(main_vp->Pos.x + main_vp->Size.x + 10, main_vp->Pos.y);
		ImVec2 editor_size = ImVec2(1280, 960);

		ImGui::SetNextWindowPos(editor_pos, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(editor_size, ImGuiCond_FirstUseEver);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGuiWindowFlags editor_flags = ImGuiWindowFlags_NoDocking;

		Bool editor_open = true;
		ImGui::Begin("MXRender Editor", &editor_open, editor_flags);
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("FPS: %.2f (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);

		ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);

		for (auto& panel : panels)
		{
			panel->Draw();
		}

		ImGui::End();

		if (!editor_open) show_editor = false;
	}

	ImGui::Render();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	return ImGui::GetDrawData();
}

//  Render thread: record ImGui GPU commands to CommandList
void EditorUI::DrawFrame_Render(ImDrawData* draw_data, RHI::CommandList* cmd)
{
	if (!draw_data || !cmd) return;

	Vector<RHI::Texture*> rtvs;
	rtvs = { m_viewport->GetCurrentBackBufferRTV() };
	cmd->SetRenderTarget(rtvs, nullptr, {}, false);

	if (cmd->IsBypass()) {
		auto* vk_cb = static_cast<RHI::Vulkan::VK_CommandBuffer*>(cmd);
		ImGui_ImplVulkan_RenderDrawData(draw_data, vk_cb->GetCommandBuffer());
	} else {
		cmd->GetRecordedCommands().push_back(std::make_unique<RHICmdRenderImGui>(draw_data, ImGui::GetCurrentContext()));
	}
}

void EditorUI::Release()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void EditorUI::AddPanelUI(CONST String& name)
{
	for (auto& panel : panels)
	{
		if (panel->GetName() == name)
		{
			return;
		}
	}
	auto it =UI::BasePanel::CreatePanel(name,name);
	it ->Init();
	panels.push_back(it);
}

void EditorUI::AddPanelUI(UI::BasePanel* in_panel)
{
	in_panel->Init();
	panels.push_back(in_panel);
}

void EditorUI::OpenRanelUI(CONST String& name)
{
	for (auto& panel : panels)
	{
		if (panel->GetName() == name)
		{
			panel->is_show =true;
			return;
		}
	}
}

UI::RenderGraphPanel* EditorUI::GetRenderGraphPanel()
{
	return rg_panel;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
