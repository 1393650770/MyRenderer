#pragma once
#ifndef _DEFAULTSETTING_
#define _DEFAULTSETTING_
namespace MXRender
{
	class DefaultSetting
	{
	public:
		DefaultSetting();
		virtual ~DefaultSetting();
		int width = 1920;
		int height = 1080;
	private:

	};

}
#endif //_DEFAULTSETTING_