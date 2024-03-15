#pragma once
#ifndef _RENDERINTERFACE_
#define _RENDERINTERFACE_
#include "Core/ConstDefine.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)

MYRENDERER_BEGIN_CLASS(RenderInterface)

#pragma region MATHOD

public:
	RenderInterface() MYDEFAULT;
	VIRTUAL ~RenderInterface() MYDEFAULT;

	VIRTUAL void BeginRender() PURE;
	VIRTUAL void EndRender() PURE;
	VIRTUAL void BeginFrame() PURE;
	VIRTUAL void OnFrame() PURE;
	VIRTUAL void EndFrame() PURE;

protected:

private:

#pragma endregion

#pragma region MEMBER

public:

protected:

private:

#pragma endregion

MYRENDERER_END_CLASS




MYRENDERER_END_NAMESPACE

#endif

