#pragma once
#ifndef _RENDERGRAPHBLACKBOARD_
#define _RENDERGRAPHBLACKBOARD_

#include "Core/ConstDefine.h"
#include <any>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Passes can Set/Get named values that persist for the graph's lifetime.
// Typical use: view/projection matrices, frame index, per-frame constants
// that don't need to travel through GPU resource edges.
MYRENDERER_BEGIN_CLASS(RenderGraphBlackboard)
public:
	template<typename T>
	void METHOD(Set)(CONST String& key, T&& value)
	{
		m_data[key] = std::forward<T>(value);
	}

	template<typename T>
	T* METHOD(Get)(CONST String& key)
	{
		auto it = m_data.find(key);
		if (it == m_data.end()) return nullptr;
		return std::any_cast<T>(&it->second);
	}

	template<typename T>
	T METHOD(GetOrDefault)(CONST String& key, CONST T& default_value)
	{
		T* p = Get<T>(key);
		return p ? *p : default_value;
	}

	Bool METHOD(Has)(CONST String& key) CONST
	{
		return m_data.count(key) > 0;
	}

	void METHOD(Clear)()
	{
		m_data.clear();
	}

private:
	Map<String, std::any> m_data;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
