#include "DeferRender.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>

#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../RHI/Vulkan/VK_Viewport.h"
#include "../RHI/Vulkan/VK_SwapChain.h"
#include "../Utils/Singleton.h"
#include "DefaultSetting.h"
#include "Pass/MainCameraRenderPass.h"
#include "Pass/MeshPass.h"
#include "Pass/UIPass.h"
#include "Pass/PreComputeIblPass.h"
#include "vulkan/vulkan_core.h"
#include "../Logic/TaskScheduler.h"
#include<Windows.h>
#include "../UI/Window_UI.h"


MXRender::DeferRender::DeferRender()
{

}

MXRender::DeferRender::~DeferRender()
{
}

void MXRender::DeferRender::run(std::weak_ptr <VK_GraphicsContext> context)
{
	if(context.expired()) return ;


	context.lock()->pre_pass();

	main_camera_pass->begin_pass(context.lock().get());



	mesh_pass->draw(context.lock().get());
	main_camera_pass->draw(context.lock().get());

	ui_pass->draw(context.lock().get());

	main_camera_pass->end_pass(context.lock().get());


	context.lock()->submit();

	//TaskGraph* task_graph = new TaskGraph();
	//task_graph->add_task_node(0, "", task_graph->get_task([]() {std::cout << "0" << std::endl; }), {});
	//task_graph->add_task_node(1, "", task_graph->get_task([]() {std::cout << "1" << std::endl; }), { 0,2 });
	//task_graph->add_task_node(2, "", task_graph->get_task([]() {std::cout << "2" << std::endl; }), { 0 });
	//task_graph->add_task_node(3, "", task_graph->get_task([]() {std::cout << "3" << std::endl; }), { 0,1 });
	//task_graph->add_task_node(4, "", task_graph->get_task([]() {std::cout << "4" << std::endl; }), { 0 });
	//task_graph->add_task_node(5, "", task_graph->get_task([]() {std::cout << "5" << std::endl; }), { 0 });
	//task_graph->add_task_node(6, "", task_graph->get_task([]() {std::cout << "6" << std::endl; }), { 0 });
	//{
	//	auto it = Singleton<DefaultSetting>::get_instance().thread_system->excute_task_graph(task_graph);
	//	it.wait();
	//}
	//std::cout<<"-----"<<std::endl;
	//{
	//	auto it = Singleton<DefaultSetting>::get_instance().thread_system->excute_task_graph(task_graph);
	//	it.wait();
	//}
	//delete task_graph;
}

void MXRender::DeferRender::init(std::weak_ptr <VK_GraphicsContext> context,GLFWwindow* window, WindowUI* window_ui)
{ 

	PassInfo pass_info;
	VKPassCommonInfo other_info;
	other_info.context= context;
	main_camera_pass = std::make_shared<MainCamera_RenderPass>();
	mesh_pass=std::make_shared<Mesh_RenderPass>();
	ui_pass = std::make_shared<UI_RenderPass>();
	precomputeibl_pass = std::make_shared<PreComputeIBL_RenderPass>();

	main_camera_pass->initialize(pass_info, &other_info);
	//context.lock()->mesh_pass=main_camera_pass->get_render_pass();
	other_info.render_pass=main_camera_pass->get_render_pass();
	mesh_pass->initialize(pass_info, &other_info);
	ui_pass->initialize(pass_info,&other_info);
	precomputeibl_pass->initialize(pass_info, &other_info);

	ui_pass->initialize_ui_renderbackend(window_ui);
	window_ui->initialize_resource();

	precomputeibl_pass->draw(context.lock().get());
	precomputeibl_pass->print(context.lock().get());
}
