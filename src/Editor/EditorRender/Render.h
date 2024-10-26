
#pragma once
#ifndef _EDITORRENDERPIPELINE_
#define _EDITORRENDERPIPELINE_

#include <imgui.h>

#include "Core/ConstDefine.h"
#include "Render/RenderInterface.h"
#include "Render/Core/RenderGraph.h"
#include "EditorUI.h"


MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Application)
class Window;

MYRENDERER_BEGIN_CLASS_WITH_DERIVE(EditorRenderPipeline, public MXRender::RenderInterface)


#pragma region METHOD
public:
	EditorRenderPipeline(MXRender::Application::Window* in_window);
	VIRTUAL ~EditorRenderPipeline() MYDEFAULT;
	EditorRenderPipeline() MYDEFAULT;

	VIRTUAL void METHOD(BeginRender)() OVERRIDE FINAL;
	VIRTUAL void METHOD(EndRender)() OVERRIDE FINAL;
	VIRTUAL void METHOD(BeginFrame)() OVERRIDE FINAL;
	VIRTUAL void METHOD(OnFrame)() OVERRIDE FINAL;
	VIRTUAL void METHOD(EndFrame)() OVERRIDE FINAL;

	Application::Window* METHOD(GetWindow)();
protected:

private:


#pragma endregion

#pragma region MEMBER
public:

protected:
	Render::RenderGraph graph;
	Window* window;
	EditorUI editor_ui;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_EDITORRENDERPIPELINE_