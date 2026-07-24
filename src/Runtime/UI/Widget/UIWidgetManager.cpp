#include "UIWidgetManager.h"
#include "UIWidget.h"

#include <algorithm>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(Widget)

UIWidgetManager::~UIWidgetManager()
{
	// Destroy in reverse order (children destroyed before parents)
	while (!m_widgets.empty())
	{
		m_widgets.back()->Destroy();
		m_widgets.pop_back();
	}
	m_widget_by_name.clear();
}

void UIWidgetManager::RegisterWidget(UniquePtr<UIWidget> widget)
{
	// Store before Initialize so FindWidget works during init
	if (!widget->GetModelName().empty())
		m_widget_by_name[widget->GetModelName()] = widget.get();
	m_widgets.push_back(std::move(widget));
}

void UIWidgetManager::SynchronizeAll()
{
	for (auto& w : m_widgets)
	{
		w->SynchronizeProperties();
	}
}

void UIWidgetManager::DestroyWidget(UIWidget* widget)
{
	auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
		[widget](const UniquePtr<UIWidget>& w) { return w.get() == widget; });
	if (it != m_widgets.end())
	{
		(*it)->Destroy();
		m_widget_by_name.erase((*it)->GetModelName());
		m_widgets.erase(it);
	}
}

UIWidget* UIWidgetManager::FindWidget(CONST String& model_name) CONST
{
	auto it = m_widget_by_name.find(model_name);
	return (it != m_widget_by_name.end()) ? it->second : nullptr;
}

MYRENDERER_END_NAMESPACE // Widget
MYRENDERER_END_NAMESPACE // UI
MYRENDERER_END_NAMESPACE // MXRender
