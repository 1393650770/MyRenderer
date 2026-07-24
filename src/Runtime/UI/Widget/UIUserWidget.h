#pragma once
#ifndef _UIUSERWIDGET_
#define _UIUSERWIDGET_

#include "Core/ConstDefine.h"
#include "UI/UIHandleTypes.h"
#include "UI/Widget/UIWidget.h"

#include <unordered_map>
#include <functional>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(Widget)

/**
 * A composite widget that owns an RML document in addition to a data model.
 *
 * The developer sets m_rml_path in the constructor.  During Initialize(),
 * the document is loaded, shown, and child widgets are auto-matched.
 */
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(UIUserWidget, public UIWidget)
#pragma region METHOD
public:
	UIUserWidget() MYDEFAULT;
	VIRTUAL ~UIUserWidget();

	VIRTUAL void METHOD(Initialize)(UIWidgetManager* manager) OVERRIDE;
	VIRTUAL void METHOD(Destroy)() OVERRIDE;

	// --- Document control ---
	void METHOD(Show)();
	void METHOD(Hide)();
	void METHOD(Close)();

	UIDocHandle METHOD(GetDocumentHandle)() CONST { return m_doc_handle; }

	// --- RML path ---
	void METHOD(SetRmlPath)(CONST String& path) { m_rml_path = path; }
	CONST String& METHOD(GetRmlPath)() CONST { return m_rml_path; }

	// --- Child widget access ---
	template<typename T>
	T* METHOD(GetChild)(CONST String& name) CONST
	{
		auto it = m_child_widgets.find(name);
		if (it != m_child_widgets.end())
			return dynamic_cast<T*>(it->second.get());
		return nullptr;
	}

protected:
	/// RML document path.  Must be set before Initialize() (via constructor
	/// or CreateUserWidget).  Relative to the working directory.
	String        m_rml_path;

	/// If true, Show() is called automatically at the end of Initialize().
	bool          m_auto_show = true;

	/// Child widgets matched by name during Initialize().  Filled by
	/// MatchChildWidgets() override.
	Map<String, UniquePtr<UIWidget>> m_child_widgets;

private:
	UIDocHandle   m_doc_handle;
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE // Widget
MYRENDERER_END_NAMESPACE // UI
MYRENDERER_END_NAMESPACE // MXRender

#endif // !_UIUSERWIDGET_
