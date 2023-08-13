#pragma once
#ifndef _VIEWPORT_
#define _VIEWPORT_
#include "../Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(Viewport,  public RenderResource)
#pragma region METHOD
public:
	Viewport() DEFAULT;
	VIRTUAL ~Viewport() DEFAULT;
protected:

private:

#pragma endregion


#pragma region MEMBER

#pragma endregion



MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
