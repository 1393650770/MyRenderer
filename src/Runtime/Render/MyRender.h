#pragma once
#ifndef _MYRENDER_
#define _MYRENDER_

namespace MXRender
{
	class MyRender
	{
	public:
		MyRender();
		virtual ~MyRender();
		virtual void run() = 0;
	private:

	};

}
#endif // !_MYRENDER_

