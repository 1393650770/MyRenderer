#pragma once
#include "ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_CLASS(IObject)
public:
	IObject()MYDEFAULT;
	VIRTUAL ~IObject()MYDEFAULT;
	void AddRef() { ++ref_count; }
	VIRTUAL void Release()=0;
protected:
	Bool is_valid{ false };
	UInt32 ref_count{ 1 };
private:

MYRENDERER_END_CLASS
MYRENDERER_END_NAMESPACE
