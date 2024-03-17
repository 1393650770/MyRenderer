
#pragma once
#ifndef _EDITORUI_
#define _EDITORUI_

#include <imgui.h>

#include "Core/ConstDefine.h"
#include "Render/RenderInterface.h"
#include "Render/Core/RenderGraph.h"
#include "UI/UIBase.h"



MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class BasePanel;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Application)
class Window;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(EditorUI,public UI::UIBase)


#pragma region METHOD
public:
	VIRTUAL ~EditorUI() MYDEFAULT;
	EditorUI() MYDEFAULT;

	void METHOD(Init)(Window* in_window);
	void METHOD(AddPass)(Render::RenderGraph* in_graph);
	void METHOD(Release)();
protected:
	void AddPanelUI(CONST String& name);
	void AddPanelUI(UI::BasePanel* in_panel);
	void OpenRanelUI(CONST String& name);
private:


#pragma endregion

#pragma region MEMBER
public:
	Window* window = nullptr;
protected:
	Vector<UI::BasePanel*> panels;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_EDITORUI_