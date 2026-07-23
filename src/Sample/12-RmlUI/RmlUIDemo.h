#pragma once
#ifndef _RMLUIDEMO_
#define _RMLUIDEMO_

#include "Core/ConstDefine.h"
#include "Application/SampleApp.h"
#include "UI/UIHandleTypes.h"
#include "UI/RmlUI/Binding/RmlUIDataModelMacros.h"

// No RmlUI headers here — everything is behind RmlUIManager

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)
class RmlUIManager;
namespace Generated { template<typename T> struct RmlBindingAccessor; }
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
	// Demo data — public so Generated::BindRmlUIDemoApp can access them
	int m_hp = 100;
	int m_score = 0;
	float m_timer = 0.0f;
	int m_prev_hp = 100;
	int m_prev_score = 0;
	float m_prev_timer = 0.0f;
protected:
private:
	MXRender::UI::RmlUI::RmlUIManager* m_manager = nullptr;

	// Typed handles (ResourceHandle<Tag>) — no RmlUI types exposed
	MXRender::UI::RmlUI::RmlModelHandle m_hud_model;
	MXRender::UI::RmlUI::RmlDocHandle   m_hud_doc;
#pragma endregion
};

RMLUI_BIND_FIELD_AS(RmlUIDemoApp, m_hp, "hp");
RMLUI_BIND_FIELD_AS(RmlUIDemoApp, m_score, "score");
RMLUI_BIND_FIELD_AS(RmlUIDemoApp, m_timer, "timer");

#endif // !_RMLUIDEMO_
