#pragma once
#ifndef _RENDER_
#define _RENDER_

namespace MXRender
{
	class Render
	{
	public:
		Render();
		virtual ~Render();
		virtual void run() = 0;
	private:

	};

}
#endif // !_RENDER_

