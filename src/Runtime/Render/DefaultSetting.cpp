#include"DefaultSetting.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Input/InputSystem.h"

MXRender::DefaultSetting::DefaultSetting()
{
	context = std::make_shared<VK_GraphicsContext>();
	gameobject_manager=std::make_shared<GameObjectManager>(context.get());
	input_system=std::make_shared<InputSystem>();
	input_system->cur_controller_object=&gameobject_manager->main_camera;
}

MXRender::DefaultSetting::~DefaultSetting()
{
	gameobject_manager->destroy_object_list(context.get());
}
