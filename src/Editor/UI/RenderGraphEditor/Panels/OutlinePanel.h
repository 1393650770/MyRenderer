#pragma once
#ifndef _OUTLINEPANEL_
#define _OUTLINEPANEL_

#include "UI/BasePanel.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class RenderGraphPanel;
class BaseNode;

// Tree-view outline showing all Passes and Resources in the graph.
// Click to select a node (syncs with PropertiesPanel and graph view).
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(OutlinePanel, public BasePanel)

#pragma region METHOD
public:
	VIRTUAL ~OutlinePanel() MYDEFAULT;
	OutlinePanel() MYDEFAULT;
	OutlinePanel(CONST String& in_name, Bool in_show = true);
	OutlinePanel(CONST OutlinePanel& other) MYDELETE;
	OutlinePanel(OutlinePanel&& other) MYDELETE;
	OutlinePanel& operator=(CONST OutlinePanel& other) MYDELETE;
	OutlinePanel& operator=(OutlinePanel&& other) MYDELETE;

	static CONST String METHOD(GetTypeName)();

	VIRTUAL void METHOD(Init)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Update)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Draw)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Release)() OVERRIDE FINAL;

	// Set the data source (RenderGraphPanel that owns nodes/links)
	void METHOD(SetDataSource)(RenderGraphPanel* rgp);

protected:
	void DrawTreeNode(BaseNode* node, Bool is_pass);
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	RenderGraphPanel* rg_panel = nullptr;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_OUTLINEPANEL_
