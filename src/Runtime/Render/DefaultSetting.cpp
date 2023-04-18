#include"DefaultSetting.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Input/InputSystem.h"
#include "../Logic/TaskScheduler.h"
#include "Pass/PipelineShaderObject.h"
#include "TextureManager.h"
MXRender::DefaultSetting::DefaultSetting()
{
	context = std::make_shared<VK_GraphicsContext>();
	gameobject_manager=std::make_shared<GameObjectManager>(context.get());
	input_system=std::make_shared<InputSystem>();
	thread_system=std::make_shared<ThreadPool>();
	texture_manager=std::make_shared<TextureManager>();
	task_graph = std::make_shared<TaskGraph>();
	thread_system->init(std::thread::hardware_concurrency(), std::thread::hardware_concurrency());
	input_system->cur_controller_object=&gameobject_manager->main_camera;
	material_system= std::make_shared<MaterialSystem>();

}

MXRender::DefaultSetting::~DefaultSetting()
{
	material_system->destroy();
	gameobject_manager->destroy_object_list(context.get());
}
