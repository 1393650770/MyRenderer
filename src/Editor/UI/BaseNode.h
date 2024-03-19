
#pragma once
#ifndef _BASENODE_
#define _BASENODE_

#include <imgui.h>

#include "Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(BaseNode)
#pragma region METHOD
public:
	VIRTUAL ~BaseNode() MYDEFAULT;
	BaseNode() MYDEFAULT;
	BaseNode(CONST String& in_name, Bool in_show = true);
	BaseNode(CONST BaseNode& other) MYDELETE;
	BaseNode(BaseNode&& other) MYDELETE;
	BaseNode& operator=(CONST BaseNode& other) MYDELETE;
	BaseNode& operator=(BaseNode&& other) MYDELETE;


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
#endif // !_BASENODE_