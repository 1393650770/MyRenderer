#pragma once
#ifndef _UIWIDGETMANAGER_
#define _UIWIDGETMANAGER_

#include "Core/ConstDefine.h"
#include "UI/Widget/UIWidgetBinding.h"  // for rttr::type in factory templates

#include <vector>
#include <map>
#include <memory>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(Widget)

class UIWidget;

/**
 * Central lifecycle manager for UIWidget instances.
 *
 * One instance per application, typically created in OnInitScene().
 * Depends only on UIManager (global singleton) — no RmlUI backend
 * headers are included.
 */
MYRENDERER_BEGIN_CLASS(UIWidgetManager)
#pragma region METHOD
public:
	UIWidgetManager() MYDEFAULT;
	~UIWidgetManager();  // Destroys all widgets in reverse order

	/// Create a plain data-model widget.  T must derive UIWidget.
	/// Returns raw pointer (lifetime owned by the manager).
	template<typename T, typename... Args>
	T* METHOD(CreateWidget)(Args&&... args)
	{
		auto widget = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = widget.get();
		RegisterWidget(std::move(widget));
		ptr->SetConcreteType(rttr::type::get<T>());
		ptr->Initialize(this);
		return ptr;
	}

	/// Create a composite widget that loads an .rml document.
	/// T must derive UIUserWidget.
	template<typename T>
	T* METHOD(CreateUserWidget)(CONST String& rml_path)
	{
		auto widget = std::make_unique<T>();
		T* ptr = widget.get();
		ptr->SetRmlPath(rml_path);
		RegisterWidget(std::move(widget));
		ptr->SetConcreteType(rttr::type::get<T>());
		ptr->Initialize(this);
		return ptr;
	}

	/// Per-frame: flush cross-thread commands and diff all widget bindings.
	/// Must be called on the main thread.
	void METHOD(SynchronizeAll)();

	/// Remove and destroy a single widget.
	void METHOD(DestroyWidget)(UIWidget* widget);

	/// Find a widget by model name.
	UIWidget* METHOD(FindWidget)(CONST String& model_name) CONST;

protected:
private:
	void METHOD(RegisterWidget)(UniquePtr<UIWidget> widget);

	Vector<UniquePtr<UIWidget>> m_widgets;
	Map<String, UIWidget*>      m_widget_by_name;
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

#endif // !_UIWIDGETMANAGER_
