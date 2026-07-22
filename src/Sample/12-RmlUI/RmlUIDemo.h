#pragma once
#ifndef _RMLUIDEMO_
#define _RMLUIDEMO_

#include "Core/ConstDefine.h"
#include "Application/SampleApp.h"
#include "UI/UIHandleTypes.h"

// No RmlUI headers here — everything is behind RmlUIManager

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)
class RmlUIManager;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

class RmlUIDemoApp : public MXRender::Application::SampleApp
{
#pragma region METHOD
public:
	RmlUIDemoApp();
	VIRTUAL ~RmlUIDemoApp();

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

	// Typed handles (ResourceHandle<Tag>) — no RmlUI types exposed
	MXRender::UI::RmlUI::RmlModelHandle m_hud_model;
	MXRender::UI::RmlUI::RmlDocHandle   m_hud_doc;

	// Demo data
	int m_hp = 100;
	int m_score = 0;
	float m_timer = 0.0f;
#pragma endregion
};

#endif // !_RMLUIDEMO_
