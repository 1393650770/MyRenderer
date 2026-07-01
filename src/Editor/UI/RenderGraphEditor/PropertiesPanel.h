#pragma once
#ifndef _PROPERTIESPANEL_
#define _PROPERTIESPANEL_

#include "UI/BasePanel.h"
#include "RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class RenderGraphPassNode;
class RenderGraphResourceNode;

// Properties inspector panel for the RenderGraph editor.
// Displays and allows editing of the currently selected node's properties.
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(PropertiesPanel, public BasePanel)

#pragma region METHOD
public:
	VIRTUAL ~PropertiesPanel() MYDEFAULT;
	PropertiesPanel() MYDEFAULT;
	PropertiesPanel(CONST String& in_name, Bool in_show = true);
	PropertiesPanel(CONST PropertiesPanel& other) MYDELETE;
	PropertiesPanel(PropertiesPanel&& other) MYDELETE;
	PropertiesPanel& operator=(CONST PropertiesPanel& other) MYDELETE;
	PropertiesPanel& operator=(PropertiesPanel&& other) MYDELETE;

	static CONST String METHOD(GetTypeName)();

	VIRTUAL void METHOD(Init)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Update)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Draw)() OVERRIDE FINAL;
	VIRTUAL void METHOD(Release)() OVERRIDE FINAL;

	// Set the currently selected node (shared selection state)
	void METHOD(SetSelectedNode)(BaseNode* node);
	BaseNode* METHOD(GetSelectedNode)() CONST { return selected_node; }

	// Set data source RenderGraphPanel (for reading selection)
	void METHOD(SetDataSource)(class RenderGraphPanel* rgp) { rg_panel = rgp; }

protected:
	void METHOD(DrawPassProperties)(RenderGraphPassNode* pass_node);
	void METHOD(DrawResourceProperties)(RenderGraphResourceNode* res_node);
	void METHOD(DrawEmptySelection)();

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	BaseNode* selected_node = nullptr;
	class RenderGraphPanel* rg_panel = nullptr;

	// Texture format name cache for combo box
	Vector<String> texture_format_names;
	Bool format_names_initialized = false;
	void InitFormatNames();

private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_PROPERTIESPANEL_
