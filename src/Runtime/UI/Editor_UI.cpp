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
#include "../Logic/GameObject.h"
#include "../Logic/Object.h"
#include "../Logic/Component/InputComponent.h"
#include "../RHI/Vulkan/VK_Texture.h"

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
			if (ImGui::BeginMenu("OtherSetting"))
			{

			    bool is_open_window=false;
				if (ImGui::Begin("OtherSetting", &is_open_window))
				{
					ImGui::Checkbox("Dispatch", &Singleton<DefaultSetting>::get_instance().is_enable_dispatch);
					ImGui::Checkbox("GPUDriven", &Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven);
					ImGui::Checkbox("DebugLoop", &(Singleton<DefaultSetting>::get_instance().is_enable_debug_loop));
					ImGui::End();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Exit"))
			{
				Singleton<DefaultSetting>::get_instance().destroy();
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

void MXRender::EditorUI::show_camera_left_detail()
{
	std::string name = "Camera";
	ImGui::BeginChild(name.c_str(), ImVec2(0, 30), true, ImGuiWindowFlags_ChildWindow);

	if (ImGui::BeginMenu(name.c_str()))
	{
		object_detail=&(Singleton<DefaultSetting>::get_instance().gameobject_manager->main_camera);
		ImGui::EndMenu();
	}
	ImGui::EndChild();
}

void MXRender::EditorUI::show_camera_right_detail()
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
}

void MXRender::EditorUI::show_one_gameobject(GameObject* gameobject)
{
	std::string name = gameobject->get_name();
	ImGuiTreeNodeFlags_ flag= gameobject->sub_objects.size()<=0? ImGuiTreeNodeFlags_Leaf: ImGuiTreeNodeFlags_None;
	//if (ImGui::BeginMenu(name.c_str()))
	//{
	//	ImGui::EndMenu();
	//}
	if(ImGui::CollapsingHeader(name.c_str(), flag))
	{ 
		if (ImGui::IsItemFocused())
		{
			object_detail = gameobject;
		}
		ImGui::Indent();
		for (auto& it : gameobject->sub_objects)
		{
			show_one_gameobject(it);
		}
		ImGui::Unindent();
	}
}

void MXRender::EditorUI::show_one_directory(const std::string& selected_asset_folder)
{
	ImGui::SameLine();

	ImGui::BeginChild((selected_asset_folder +" Asset Details").c_str());
	unsigned int allready_show_num_in_one_line=0;
	ImGui::Separator();
	if (selected_asset_folder=="Material")
	{
		MaterialSystem* material_system= (Singleton<DefaultSetting>::get_instance().material_system.get());
		for (auto& it: material_system->materials)
		{
			show_one_file(it.first,selected_asset_folder,allready_show_num_in_one_line);
		}
	}
	else if (selected_asset_folder == "Mesh")
	{
		auto& mesh_dir= (Singleton<DefaultSetting>::get_instance().gameobject_manager->get_mesh_cache());
		for (auto& it : mesh_dir)
		{
			show_one_file(it.first, selected_asset_folder, allready_show_num_in_one_line);
		}
	}
	else if (selected_asset_folder == "Texture")
	{
		auto& texture_dir = (Singleton<DefaultSetting>::get_instance().gameobject_manager->get_texture_cache());
		for (auto& it : texture_dir)
		{
			show_one_file(it.first, selected_asset_folder, allready_show_num_in_one_line);
		}
	}
	else if (selected_asset_folder == "Prefabs")
	{
		auto& prefabs_dir = (Singleton<DefaultSetting>::get_instance().gameobject_manager->get_prefab_cache());
		for (auto& it : prefabs_dir)
		{
			show_one_file(it.first, selected_asset_folder, allready_show_num_in_one_line);
		}
	}



	ImGui::EndChild();
}

void MXRender::EditorUI::show_one_file(const std::string& name, const std::string& type,unsigned int& allready_show_num_in_one_line)
{


	ImGui::SameLine();
	// Show asset information

	ImGui::BeginChild((name+"Asset Details").c_str(), ImVec2(file_content_icon_size.first, file_content_icon_size.second),true);
	ImGui::BeginGroup();
	ImGui::Image(content_icon_ds, ImVec2(75, 75));
	ImGui::Separator();
	ImGui::TextWrapped(("Type: "+type).c_str());
	ImGui::TextWrapped(("Name: "+name).c_str());
	ImGui::EndGroup();
	ImGui::EndChild();
	float res_size= ImGui::GetWindowWidth();
	if (res_size-file_content_icon_size.first<allready_show_num_in_one_line* file_content_icon_size.first)
	{
		ImGui::NewLine();
		allready_show_num_in_one_line=0;
	}
	allready_show_num_in_one_line++;
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

void MXRender::EditorUI::initialize_resource()
{
	content_icon_texture = std::make_shared<VK_Texture>(ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Editor/Icon/FileIcon.png");
	directory_icon_texture = std::make_shared<VK_Texture>(ENUM_TEXTURE_TYPE::ENUM_TYPE_2D, "Resource/Editor/Icon/DirectoryIcon.png");
	content_icon_ds = ImGui_ImplVulkan_AddTexture(content_icon_texture->textureSampler, content_icon_texture->textureImageView, content_icon_texture->textureImageLayout);
	directory_icon_ds = ImGui_ImplVulkan_AddTexture(directory_icon_texture->textureSampler, directory_icon_texture->textureImageView, directory_icon_texture->textureImageLayout);
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

	show_camera_left_detail();

	for (int i = 0; i < Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list.size(); i++)
	{
		show_one_gameobject(&(Singleton<DefaultSetting>::get_instance().gameobject_manager->object_list[i]));
	}


		
	
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
	if (object_detail)
	{

		InputComponent* input = object_detail->get_component(InputComponent);
		if (input)
		{
			show_camera_right_detail();
			ImGui::End();
			return;
		}
		TransformComponent* transform = object_detail->get_component(TransformComponent);
		StaticMeshComponent* static_mesh = object_detail->get_component(StaticMeshComponent);
		std::string name = object_detail->get_name();
		glm::vec3 translation = transform->get_translation();
		glm::vec3 scale = transform->get_scale();
		glm::vec3 rotation = transform->get_rotation();

		ImGui::CollapsingHeader((name + "-Detail").c_str(), ImGuiTreeNodeFlags_Leaf);

		ImGui::Separator();

		if (ImGui::CollapsingHeader("TransformComponent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("%s", "transform translation");
			ImGui::DragFloat("translation x", &translation.r);
			ImGui::DragFloat("translation y", &translation.g);
			ImGui::DragFloat("translation z", &translation.b);

			ImGui::Separator();
			ImGui::Text("%s", "transform scale");
			//ImGui::SameLine();
			ImGui::DragFloat("scale x", &scale.r);
			ImGui::DragFloat("scale y", &scale.g);
			ImGui::DragFloat("scale z", &scale.b);
			ImGui::Separator();
			ImGui::Text("%s", "transform rotation");
			//ImGui::SameLine();
			ImGui::DragFloat("rotation x", &rotation.r);
			ImGui::DragFloat("rotation y", &rotation.g);
			ImGui::DragFloat("rotation z", &rotation.b);
		}
		if (ImGui::CollapsingHeader("MaterialComponent", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("%s", "pushconstant test");
			ImGui::DragFloat("pushconstant z", &(static_mesh->get_material()->parameters.z));
			ImGui::Separator();
		}
		transform->set_translation(translation);
		transform->set_scale(scale);
		transform->set_rotation(rotation);
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

	//ImVec2 window_size = ImGui::GetWindowSize();
	ImVec2 image_size = ImVec2(11, 11);
	//ImVec2 uv0 = ImVec2(0, 0);
	//ImVec2 uv1 = ImVec2(1, 1);



	//// Show a list of assets
	ImGui::BeginChild("Asset List", ImVec2(300, 0), true);
	ImGui::Text("Assets:");
	ImGui::Separator();



	ImGui::Image((ImTextureID)directory_icon_ds, image_size);
	ImGui::SameLine();
	if (ImGui::Selectable("Material"))
	{ 
		selected_asset_folder = "Material";
	
	}
	ImGui::Image((ImTextureID)directory_icon_ds, image_size);
	ImGui::SameLine();
	if (ImGui::Selectable("Mesh"))
	{
		selected_asset_folder = "Mesh";

	}
	ImGui::Image((ImTextureID)directory_icon_ds, image_size);
	ImGui::SameLine();
	if (ImGui::Selectable("Texture"))
	{
		selected_asset_folder = "Texture";
	}
	ImGui::Image((ImTextureID)directory_icon_ds, image_size);
	ImGui::SameLine();
	if (ImGui::Selectable("Prefabs"))
	{
		selected_asset_folder = "Prefabs";
	}

	ImGui::EndChild();

	show_one_directory(selected_asset_folder);




	ImGui::End();
}