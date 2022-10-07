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
		int width = 1900;
		int height = 1050;
	private:

	};

}
#endif //_DEFAULTSETTING_