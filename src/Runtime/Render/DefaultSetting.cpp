#include"DefaultSetting.h"
#include "../RHI/Vulkan/VK_GraphicsContext.h"


MXRender::DefaultSetting::DefaultSetting()
{
	context = std::make_shared<VK_GraphicsContext>();
}

MXRender::DefaultSetting::~DefaultSetting()
{
}
