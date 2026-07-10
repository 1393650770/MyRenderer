#pragma once
#ifndef _RENDERBINDLESSMANAGER_
#define _RENDERBINDLESSMANAGER_

#include "Core/ConstDefine.h"
#include "RenderRource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

class Texture;

// --   Platform-agnostic bindless resource manager interface
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(BindlessManager, public RenderResource)
#pragma region METHOD
public:
	VIRTUAL ~BindlessManager() MYDEFAULT;
	VIRTUAL UInt32 METHOD(AllocateTexture2DSlot)(Texture* texture) PURE;
	VIRTUAL void   METHOD(FreeTexture2DSlot)(UInt32 index) PURE;
	VIRTUAL UInt32 METHOD(AllocateTextureCubeSlot)(Texture* texture) PURE;
	VIRTUAL void   METHOD(FreeTextureCubeSlot)(UInt32 index) PURE;
	VIRTUAL Bool   METHOD(IsEnabled)() CONST PURE;
#pragma endregion

#pragma region MEMBER
public:
private:
protected:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
