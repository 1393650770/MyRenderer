#include"DefaultSetting.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Input/InputSystem.h"
#include "../Logic/TaskScheduler.h"
MXRender::DefaultSetting::DefaultSetting()
{
	context = std::make_shared<VK_GraphicsContext>();
	gameobject_manager=std::make_shared<GameObjectManager>(context.get());
	input_system=std::make_shared<InputSystem>();
	thread_system=std::make_shared<ThreadPool>();
	thread_system->init(std::thread::hardware_concurrency(), std::thread::hardware_concurrency());
	input_system->cur_controller_object=&gameobject_manager->main_camera;
}

MXRender::DefaultSetting::~DefaultSetting()
{
	gameobject_manager->destroy_object_list(context.get());
}
