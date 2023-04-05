#include "Editor_UI.h"
#include <array>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Render/DefaultSetting.h"
#include "../Utils/Singleton.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Component/TransformComponent.h"


inline void on_window_content_scale_update(float scale)
{
#if defined(__GNUC__) && defined(__MACH__)
	float font_scale = fmaxf(1.0f, scale);
	ImGui::GetIO().FontGlobalScale = 1.0f / font_scale;
#endif
	// TOOD: Reload fonts if DPI scale is larger than previous font loading DPI scale
}

inline void on_window_content_scale_callback(GLFWwindow* window, float x_scale, float y_scale)
{
	on_window_content_scale_update(fmaxf(x_scale, y_scale));
}

void MXRender::EditorUI::show_editor_top_ui()
{
	ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_DockSpace;
	ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
		ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
	std::array<int, 2> window_size={ Singleton<DefaultSetting>::get_instance().width, Singleton<DefaultSetting>::get_instance().height};
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
		ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.25f, nullptr, &left_other);

		ImGuiID left_game_engine;
		ImGuiID left_asset =
			ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.25f, nullptr, &left_game_engine);

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

void MXRender::EditorUI::show_camera_detail()
{
	std::string name = "Camera";
	ImGui::BeginChild(name.c_str(), ImVec2(0, 30), true, ImGuiWindowFlags_ChildWindow);

	if (ImGui::BeginMenu(name.c_str()))
	{
		
		ImGui::Text("%s", "Camera Position");
		//ImGui::SameLine();
		ImGui::Text("Camera Position x : %f - Camera Position y : %f - Camera Position z : %f", 
		(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_position().r),
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_position().g,
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_position().b);
		ImGui::Text("Camera Direction x : %f - Camera Direction y : %f - Camera Direction z : %f",
			(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_direction().r),
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_direction().g,
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_direction().b);
		ImGui::Text("Camera Right x : %f - Camera Right y : %f - Camera Right z : %f",
			(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_right().r),
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_right().g,
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_right().b);
		ImGui::Text("Camera Up x : %f - Camera Up y : %f - Camera Up z : %f",
			(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_up().r),
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_up().g,
			Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_up().b);
		ImGui::DragFloat("Camera Move Speed", &Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_can_change_move_speed());
		ImGui::DragFloat("Camera Near Plane", &Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_can_change_near_plane());
		ImGui::DragFloat("Camera Far Plane", &Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_can_change_far_plane());
		ImGui::DragFloat("Camera Fov", &Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera.get_can_change_fov());
		ImGui::EndMenu();
	}
	ImGui::EndChild();
}

MXRender::EditorUI::EditorUI()
{

}

MXRender::EditorUI::~EditorUI()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void MXRender::EditorUI::initialize(WindowUIInitInfo* init_info)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();



	EditorUIInitInfo* editui_info=static_cast<EditorUIInitInfo*>(init_info);
	context=editui_info->context;
	VK_GraphicsContext* vk_context=dynamic_cast<VK_GraphicsContext*>(editui_info->context);

	float x_scale, y_scale;
	glfwGetWindowContentScale(vk_context->get_window(), &x_scale, &y_scale);
	float content_scale = fmaxf(1.0f, fmaxf(x_scale, y_scale));
	on_window_content_scale_update(content_scale);


	glfwSetWindowContentScaleCallback(vk_context->get_window(), on_window_content_scale_callback);

}

void MXRender::EditorUI::pre_render()
{
	show_editor_top_ui();
	show_editor_left_ui();
	show_editor_right_ui();
	show_editor_down_ui();
	show_center_main_window();
}

void MXRender::EditorUI::show_editor_left_ui()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();



	if (!ImGui::Begin("World Objects", nullptr, window_flags))
	{
		ImGui::End();
		return;
	}


	//if (ImGui::Selectable(nullptr,nullptr,0,ImVec2(2,2)))
	//{


	//}
		
	
	ImGui::End();
}

void MXRender::EditorUI::show_editor_right_ui()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();



	if (!ImGui::Begin("Detail", nullptr, window_flags))
	{
		ImGui::End();
		return;
	}


	show_camera_detail();

	for (int i = 0; i < Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list.size(); i++)
	{
		TransformComponent* transform= Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list[i].get_transform();
		std::string name = Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list[i].get_name();
		glm::vec3 translation = transform->get_translation();
		glm::vec3 scale = transform->get_scale();
		glm::vec3 rotation = transform->get_rotation();
		ImGui::BeginChild(name.c_str(), ImVec2(0, 30), true, ImGuiWindowFlags_ChildWindow);

		if (ImGui::BeginMenu(name.c_str()))
		{ 

			ImGui::Text("%s", "transform translation");
			//ImGui::SameLine();
			ImGui::DragFloat("translation x", &translation.r);
			ImGui::DragFloat("translation y", &translation.g);
			ImGui::DragFloat("translation z", &translation.b);
			ImGui::Text("%s", "transform scale");
			//ImGui::SameLine();
			ImGui::DragFloat("scale x", &scale.r);
			ImGui::DragFloat("scale y", &scale.g);
			ImGui::DragFloat("scale z", &scale.b);
			ImGui::Text("%s", "transform rotation");
			//ImGui::SameLine();
			ImGui::DragFloat("rotation x", &rotation.r);
			ImGui::DragFloat("rotation y", &rotation.g);
			ImGui::DragFloat("rotation z", &rotation.b);
			transform->set_translation(translation);
			transform->set_scale(scale);
			transform->set_rotation(rotation);
			ImGui::EndMenu();
		}
		ImGui::EndChild();
	}


	ImGui::End();
}



void MXRender::EditorUI::show_center_main_window()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();


	if (!ImGui::Begin("Game Engine", nullptr, window_flags))
	{
		ImGui::End();
		return;
	}



	auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();

	glm::vec2 new_window_pos = { 0.0f, 0.0f };
	glm::vec2 new_window_size = { 0.0f, 0.0f };
	new_window_pos.x = ImGui::GetWindowPos().x;
	new_window_pos.y = ImGui::GetWindowPos().y + menu_bar_rect.Min.y;
	new_window_size.x = ImGui::GetWindowSize().x;
	new_window_size.y = ImGui::GetWindowSize().y - menu_bar_rect.Min.y;

	VK_GraphicsContext* vk_context = dynamic_cast<VK_GraphicsContext*>(context);
	if (vk_context)
	{
		vk_context->viewport.x = new_window_pos.x;
		vk_context->viewport.y = new_window_pos.y;
		vk_context->viewport.width= new_window_size.x;
		vk_context->viewport.height= new_window_size.y;
	}

	ImGui::End();
}


void MXRender::EditorUI::show_editor_down_ui()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();



	if (!ImGui::Begin("File Content", nullptr, window_flags))
	{
		ImGui::End();
		return;
	}


	//if (ImGui::Selectable(nullptr,nullptr,0,ImVec2(2,2)))
	//{


	//}


	ImGui::End();
}