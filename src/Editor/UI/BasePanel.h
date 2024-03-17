
#pragma once
#ifndef _BASEPANNEL_
#define _BASEPANNEL_

#include <imgui.h>

#include "Core/ConstDefine.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)


class BasePanel;
class PanelRegister;

typedef BasePanel* (*PanelCreateFunction)(CONST String& in_name, Bool in_show);


MYRENDERER_BEGIN_CLASS(BasePanel)
friend class PanelRegister;

#pragma region METHOD
public:
	VIRTUAL ~BasePanel() MYDEFAULT;
	BasePanel() MYDEFAULT;
	BasePanel(CONST String& in_name, Bool in_show = true);
	BasePanel(CONST BasePanel& other) MYDELETE;
	BasePanel(BasePanel&& other) MYDELETE;
	BasePanel& operator=(CONST BasePanel& other) MYDELETE;
	BasePanel& operator=(BasePanel&& other) MYDELETE;

	VIRTUAL void METHOD(Init)() PURE;
	VIRTUAL void METHOD(Update)() PURE;
	VIRTUAL void METHOD(Draw)() PURE;
	VIRTUAL void METHOD(Release)() PURE;

	CONST String& METHOD(GetName)() CONST;
	static BasePanel* METHOD(CreatePanel)(CONST String& in_name, Bool in_show = true);
protected:
	Bool METHOD(OnBegin)(Int window_flags);
	void METHOD(OnEnd)() CONST;
private:

#pragma endregion

#pragma region MEMBER
public:
	Bool is_show = true;
protected:
	String name = "";


	static Map<String, PanelCreateFunction> panel_createfunc_map;
	friend class PanelRegister;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(PanelRegister)
public:
	PanelRegister(PanelCreateFunction CreateFunction, CONST String& in_name) :name(in_name)
	{
		BasePanel::panel_createfunc_map[name] = CreateFunction;
	}

	~PanelRegister()
	{
		BasePanel::panel_createfunc_map.erase(name);
	}
protected:
	String name;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_BASEPANNEL_