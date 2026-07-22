#pragma once
#ifndef _RMLUIEVENTLISTENER_
#define _RMLUIEVENTLISTENER_

#include "Core/ConstDefine.h"
#include <RmlUi/Core/EventListener.h>

/**
 * Base class for RmlUI event listeners.
 *
 * Inherit from this and override ProcessEvent() to handle RmlUI events
 * from .rml documents. Register via Context::AddEventListener().
 *
 * Example:
 *   class MyListener : public RmlUIEventListener {
 *       void ProcessEvent(Rml::Event& event) override {
 *           // handle event
 *       }
 *   };
 *   ctx->AddEventListener("click", new MyListener());
 */

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RmlUIEventListener, public Rml::EventListener)

#pragma region METHOD
public:
	RmlUIEventListener() MYDEFAULT;
	VIRTUAL ~RmlUIEventListener() MYDEFAULT;

	VIRTUAL void METHOD(ProcessEvent)(Rml::Event& event) OVERRIDE {}

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
MYRENDERER_END_NAMESPACE

#endif // !_RMLUIEVENTLISTENER_
