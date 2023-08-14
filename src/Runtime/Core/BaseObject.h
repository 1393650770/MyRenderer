#pragma once
#include "ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_CLASS(IObject)
public:
	IObject()DEFAULT;
	VIRTUAL ~IObject()DEFAULT;
	Int i=0;
protected:

private:

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
