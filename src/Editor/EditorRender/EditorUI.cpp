#include "EditorUI.h"
#include "Application/Window.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderTexture.h"
#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include "UI/BasePanel.h"
#include "UI/RenderGraphEditor/RenderGraphPanel.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)

using namespace Render;
using namespace RHI;
using namespace UI;
void EditorUI::Init(Window* in_window)
{
	window = in_window;
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

	GLFWwindow* glfw_window = window->GetWindow();
	CHECK_WITH_LOG(ImGui_ImplGlfw_InitForVulkan(glfw_window, true) == false, "Failed to init ImGui for Vulkan!");
	in_window->GetViewport()->AttachUiLayer(this);
	
	AddPanelUI(RenderGraphPanel::GetTypeName());
	//auto it = UI::BasePanel::CreatePanel(RenderGraphPanel::GetTypeName(),"test");

	//AddPanelUI(it);
}

void EditorUI::AddPass(RenderGraph* in_graph)
{
	struct UIPassData : public RenderGraphPassDataBase
	{
		VIRTUAL ~UIPassData()
		{
			Release();
		}
		void Release()
		{

		}
	};

	CommandList* cmd_list = RHIGetImmediateCommandList();
	in_graph->AddRenderPass<UIPassData>("UIPass", in_graph, cmd_list,
		[&](UIPassData& data, RenderGraphPassBuilder& builder, CommandList* in_cmd_list)
	{
		
	},
		[=](CONST UIPassData& data, CommandList* in_cmd_list)
	{

		Vector<RHI::ClearValue> clear_values;
		Vector<RHI::Texture*> rtvs;
		RHI::Texture* dsv=nullptr;
		rtvs = { window->GetViewport()->GetCurrentBackBufferRTV() };
		dsv = window->GetViewport()->GetCurrentBackBufferDSV();
		for (auto rtv : rtvs)
		{
			clear_values.push_back(rtv->GetTextureDesc().clear_value);
		}
		//if (dsv)
		//	clear_values.push_back(dsv->GetTextureDesc().clear_value);
		in_cmd_list->SetRenderTarget(rtvs, nullptr, clear_values, dsv != nullptr);
		cmd_list->BeginUI();

		for (auto& panel : panels)
		{
			panel->Draw();
		}

		cmd_list->EndUI();
	});

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

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE