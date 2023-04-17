#pragma once

#ifndef _EDITOR_UI_
#define _EDITOR_UI_



#include <memory>
#include "Window_UI.h"
#include "vulkan/vulkan_core.h"
#include <string>
#include <utility>

namespace MXRender { class VK_Texture; }

namespace MXRender { class GameObject; }


namespace MXRender { class Object; }

namespace MXRender { class GraphicsContext; }

namespace MXRender
{

	struct EditorUIInitInfo:public WindowUIInitInfo
	{
	public: 
		GraphicsContext* context;
	};

    //class AssetBrowser
    //{
    //public:
    //    AssetBrowser() : m_fileDialog(ImGuiFileBrowserFlags_None) {}

    //    void Draw()
    //    {
    //        ImGui::Begin("Asset Browser");

    //        // ��ʾ��ǰ�ļ���·��
    //        ImGui::Text("Current directory: %s", m_currentPath.c_str());

    //        // ��ʾ�ļ��б�
    //        for (auto& file : m_files)
    //        {
    //            ImGui::Selectable(file.c_str());
    //        }

    //        // ��ʾ�ļ����б�
    //        for (auto& dir : m_dirs)
    //        {
    //            if (ImGui::Selectable(dir.c_str()))
    //            {
    //                // ����ļ���ʱ��������ļ���
    //                m_currentPath = fs::absolute(dir).string();
    //                RefreshFiles();
    //            }
    //        }

    //        // ��ʾ�ļ������˵�
    //        if (ImGui::BeginPopupContextWindow())
    //        {
    //            if (ImGui::MenuItem("New folder"))
    //            {
    //                fs::create_directory(m_currentPath + "/New Folder");
    //                RefreshFiles();
    //            }

    //            if (ImGui::MenuItem("Delete"))
    //            {
    //                // ɾ��ѡ�е��ļ����ļ���
    //                for (auto& file : m_selectedFiles)
    //                {
    //                    fs::remove_all(m_currentPath + "/" + file);
    //                }

    //                m_selectedFiles.clear();
    //                RefreshFiles();
    //            }

    //            ImGui::EndPopup();
    //        }

    //        // ��ʾ�ļ�ѡ��Ի���
    //        m_fileDialog.Display();

    //        if (m_fileDialog.HasSelected())
    //        {
    //            // ���ѡ�����ļ���ִ�лص�����
    //            if (m_fileCallback)
    //            {
    //                m_fileCallback(m_fileDialog.GetSelected().string());
    //            }

    //            m_fileDialog.ClearSelected();
    //        }

    //        ImGui::End();
    //    }

    //    void SetFileCallback(std::function<void(std::string)> callback)
    //    {
    //        m_fileCallback = callback;
    //    }

    //private:
    //    void RefreshFiles()
    //    {
    //        m_files.clear();
    //        m_dirs.clear();

    //        // ������ǰ�ļ��У���ȡ�ļ����ļ����б�
    //        for (auto& entry : fs::directory_iterator(m_currentPath))
    //        {
    //            std::string filename = entry.path().filename().string();

    //            if (entry.is_regular_file())
    //            {
    //                m_files.push_back(filename);
    //            }
    //            else if (entry.is_directory())
    //            {
    //                m_dirs.push_back(filename);
    //            }
    //        }
    //    }

    //private:
    //    std::string m_currentPath = fs::current_path().string();
    //    std::vector<std::string> m_files;
    //    std::vector<std::string> m_dirs;
    //    std::vector<std::string> m_selectedFiles;

    //    ImGui::FileBrowser m_fileDialog;
    //    std::function<void(std::string)> m_fileCallback = nullptr;
    //};

	class EditorUI:public WindowUI
	{

	private:
        std::shared_ptr<VK_Texture> content_icon_texture;
        std::shared_ptr<VK_Texture> directory_icon_texture;

		VkDescriptorSet content_icon_ds;

		VkDescriptorSet directory_icon_ds;
		
        std::pair<float,float> file_content_icon_size= std::make_pair(95.0f, 130.f);
	protected:
		GraphicsContext* context;
		
		Object* object_detail=nullptr;
        std::string selected_asset_folder = "";

		void show_camera_left_detail();
		void show_camera_right_detail();
		void show_one_gameobject(GameObject* gameobject);

        void show_one_directory(const std::string& selected_asset_folder);
        void show_one_file(const std::string& name, const std::string& type, unsigned int& allready_show_num_in_one_line);
	public:
		EditorUI();
		virtual ~EditorUI();
		virtual void initialize(WindowUIInitInfo* init_info) ;
        virtual void initialize_resource();
		virtual void pre_render() ;
		void show_editor_left_ui();
		void show_editor_right_ui();
		void show_editor_top_ui();
		void show_editor_down_ui();
		void show_center_main_window();
	};


}
#endif
