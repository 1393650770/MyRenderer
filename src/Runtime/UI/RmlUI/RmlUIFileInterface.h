
#pragma once
#ifndef _RMLUIFILEINTERFACE_
#define _RMLUIFILEINTERFACE_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

/**
 * Rml::FileInterface implementation.
 *
 * Maps RmlUI file paths to the engine's resource loading system.
 * v1: Uses standard C++ file I/O with paths relative to working directory.
 * v2: Route through MXRender BasicFileSystem for asset bundle support.
 */
MYRENDERER_BEGIN_CLASS(RmlUIFileInterface)

#pragma region METHOD
public:
	RmlUIFileInterface();
	VIRTUAL ~RmlUIFileInterface();

	/// Install as the active FileInterface.
	void METHOD(Install)();

	/// Uninstall.
	void METHOD(Uninstall)();

protected:
private:
	class Impl;
	Impl* m_impl = nullptr;
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_RMLUIFILEINTERFACE_
