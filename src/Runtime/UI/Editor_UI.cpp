#include "Editor_UI.h"
#include <array>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"

void MXRender::EditorUI::show_editor_UI()
{
	ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_DockSpace;
	ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
		ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
	std::array<int, 2> window_size={1280,1080};
	ImGui::SetNextWindowSize(ImVec2((float)window_size[0], (float)window_size[1]), ImGuiCond_Always);

	ImGui::SetNextWindowViewport(main_viewport->ID);

	ImGui::Begin("Editor menu", nullptr, window_flags);

	ImGuiID main_docking_id = ImGui::GetID("Main Docking");
	if (ImGui::DockBuilderGetNode(main_docking_id) == nullptr)
	{
		ImGui::DockBuilderRemoveNode(main_docking_id);

		ImGui::DockBuilderAddNode(main_docking_id, dock_flags);
		ImGui::DockBuilderSetNodePos(main_docking_id,
			ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 18.0f));
		ImGui::DockBuilderSetNodeSize(main_docking_id,
			ImVec2((float)window_size[0], (float)window_size[1] - 18.0f));

		ImGuiID center = main_docking_id;
		ImGuiID left;
		ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &left);

		ImGuiID left_other;
		ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left_other);

		ImGuiID left_game_engine;
		ImGuiID left_asset =
			ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.30f, nullptr, &left_game_engine);

		ImGui::DockBuilderDockWindow("World Objects", left_asset);
		ImGui::DockBuilderDockWindow("Components Details", right);
		ImGui::DockBuilderDockWindow("File Content", left_file_content);
		ImGui::DockBuilderDockWindow("Game Engine", left_game_engine);

		ImGui::DockBuilderFinish(main_docking_id);
	}

	ImGui::DockSpace(main_docking_id);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Reload Current Level"))
			{
				
			}
			if (ImGui::MenuItem("Save Current Level"))
			{
				
			}
			if (ImGui::MenuItem("Exit"))
			{
				
				exit(0);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("World Objects", nullptr, nullptr);
			ImGui::MenuItem("Game", nullptr, nullptr);
			ImGui::MenuItem("File Content", nullptr, nullptr);
			ImGui::MenuItem("Detail", nullptr, nullptr);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::End();
}

MXRender::EditorUI::EditorUI()
{

}

MXRender::EditorUI::~EditorUI()
{

}

void MXRender::EditorUI::initialize(WindowUIInitInfo* init_info)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

}

void MXRender::EditorUI::pre_render()
{
	show_editor_UI();
}

