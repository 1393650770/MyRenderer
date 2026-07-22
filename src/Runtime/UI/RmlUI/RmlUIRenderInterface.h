#pragma once
#ifndef _RMLUIRENDERINTERFACE_
#define _RMLUIRENDERINTERFACE_

#include "Core/ConstDefine.h"

// Forward-declare Rml types to avoid exposing RmlUi headers here.
// The .cpp file includes the full RmlUi/Core/RenderInterface.h.
// We CANNOT forward-declare Rml::RenderInterface because it's a class
// with virtual methods — so we use the PIMPL pattern.
// Actually, the concrete RenderInterface override is ALL in the .cpp file.

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class UIRenderer;
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

/**
 * Adapter: Rml::RenderInterface → UI::UIRenderer
 *
 * Wraps all 20 Rml::RenderInterface virtuals, delegating each to
 * the corresponding UIRenderer method. This class is the bridge
 * between RmlUI's framework and our engine's abstract renderer.
 *
 * The actual inheritance from Rml::RenderInterface is hidden in the
 * .cpp file via an internal Impl class (PIMPL pattern) so that this
 * header doesn't pull in RmlUi headers.
 */
MYRENDERER_BEGIN_CLASS(RmlUIRenderInterface)

#pragma region METHOD
public:
	RmlUIRenderInterface(UI::UIRenderer* renderer);
	VIRTUAL ~RmlUIRenderInterface();

	/// Get the underlying Rml::RenderInterface pointer.
	/// The caller (RmlUIManager) installs this via Rml::SetRenderInterface().
	void* METHOD(GetRmlInterface)() CONST;

	/// Update the renderer reference (in case renderer is recreated).
	void METHOD(SetRenderer)(UI::UIRenderer* renderer);

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

#endif // !_RMLUIRENDERINTERFACE_
