#pragma once
#ifndef _TEXTUREASSET_
#define _TEXTUREASSET_
#include "Core/ConstDefine.h"
#include "Platform/FileSystem.h"
#include <atomic>


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
class TextureDataPayload;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Asset)


MYRENDERER_BEGIN_CLASS(TextureAsset)

#pragma region METHOD
public:
	TextureAsset() MYDEFAULT;
	TextureAsset(CONST String& path);
	~TextureAsset();

	void METHOD(LoadTexture)(CONST String& path);
	RHI::Texture* METHOD(GetTexture)();
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	RHI::Texture* texture = nullptr;
	std::atomic_bool is_loaded = false;
	RHI::TextureDataPayload* data = nullptr;
private:

#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
