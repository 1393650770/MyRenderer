#pragma once
#ifndef _DEFAULTSETTING_
#define _DEFAULTSETTING_
#include <memory>

namespace MXRender { class VK_GraphicsContext; }
namespace MXRender
{
	class DefaultSetting
	{
	public:
		std::shared_ptr <VK_GraphicsContext> context;
		DefaultSetting();
		virtual ~DefaultSetting();
		int width = 800;
		int height = 600;
	private:

	};

}
#endif //_DEFAULTSETTING_