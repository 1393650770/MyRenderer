
#pragma once
#ifndef _UIBASE_
#define _UIBASE_

#include <imgui.h>
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(UIBase)

#pragma region METHOD
public:
	VIRTUAL ~UIBase() MYDEFAULT;
	UIBase() MYDEFAULT;

	VIRTUAL void METHOD(Init)() {};
	VIRTUAL void METHOD(Update)() {};
	VIRTUAL void METHOD(Draw)() {};
	VIRTUAL void METHOD(Release)() {};
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
MYRENDERER_END_NAMESPACE


#endif // !_UIBASE_