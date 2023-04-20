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
#include "optick.h"
#include "RenderScene.h"
#include "GPUDriven.h"


MXRender::DeferRender::DeferRender()
{

}

MXRender::DeferRender::~DeferRender()
{
}

void MXRender::DeferRender::run(std::weak_ptr <VK_GraphicsContext> context,RenderScene* render_scene)
{
	if(context.expired()) return ;

	if (Singleton<DefaultSetting>::get_instance().is_enable_gpu_driven)
	{
		render_scene->gpu_driven->excute_upload_computepass(render_scene);
		render_scene->gpu_driven->execute_gpu_culling_computepass(render_scene);
		render_scene->gpu_driven->update_descriptorset();
		if (Singleton<DefaultSetting>::get_instance().is_enable_debug_loop==false)
		{
			render_scene->clear_dirty_objects();
		}

	}

	context.lock()->pre_pass();

	main_camera_pass->begin_pass(context.lock().get());


	OPTICK_PUSH("MeshDraw")
	mesh_pass->draw(context.lock().get(),render_scene);
	OPTICK_POP()

	main_camera_pass->draw(context.lock().get());
	OPTICK_PUSH("UiDraw")
	ui_pass->draw(context.lock().get());
	OPTICK_POP()
	main_camera_pass->end_pass(context.lock().get());

	OPTICK_PUSH("SubmitQueueAndWaitIdle")
	context.lock()->submit();
	OPTICK_POP()

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
