#include"DefaultSetting.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"
#include "../Logic/GameObjectManager.h"
#include "../Logic/Input/InputSystem.h"

MXRender::DefaultSetting::DefaultSetting()
{
	context = std::make_shared<VK_GraphicsContext>();
	gameobject_manager=std::make_shared<GameObjectManager>();
	input_system=std::make_shared<InputSystem>();
}

MXRender::DefaultSetting::~DefaultSetting()
{
	gameobject_manager->destroy_object_list(context.get());
}
