#pragma once
#ifndef _RMLUIDEMO_
#define _RMLUIDEMO_

#include "Core/ConstDefine.h"
#include "Application/SampleApp.h"

// Forward declarations
namespace Rml {
class DataModelHandle;
}

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)
class RmlUIManager;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

/**
 * Sample 12: RmlUI Demo — game UI with retained-mode framework.
 *
 * Demonstrates:
 * - RmlUI initialization and RDG pass integration
 * - Data model binding with DirtyVariable
 * - Event handling (button clicks)
 * - .rml/.rcss document loading
 */
class RmlUIDemoApp : public MXRender::Application::SampleApp
{
#pragma region METHOD
public:
	RmlUIDemoApp() MYDEFAULT;
	VIRTUAL ~RmlUIDemoApp() MYDEFAULT;

	VIRTUAL void METHOD(OnInitScene)() OVERRIDE;
	VIRTUAL void METHOD(OnShutdownScene)() OVERRIDE;
	VIRTUAL void METHOD(OnUpdate)(float dt) OVERRIDE;

protected:
private:
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	MXRender::UI::RmlUI::RmlUIManager* m_manager = nullptr;
	Rml::DataModelHandle* m_hud_handle = nullptr;

	// Demo data model
	int m_hp = 100;
	int m_score = 0;
	float m_timer = 0.0f;
#pragma endregion
};

#endif // !_RMLUIDEMO_
