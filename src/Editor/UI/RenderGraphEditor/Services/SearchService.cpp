// --   --
#include "UI/RenderGraphEditor/Services/SearchService.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include <algorithm>
#include <cctype>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

void SearchService::RebuildIndex(CONST Vector<BaseNode*>& nodes)
{
	m_index.clear();
	for (auto* node : nodes)
	{
		if (!node) continue;
		SearchEntry e;
		e.node_id = node->GetSelfID();
		e.searchable_text = node->GetName();
		for (auto* pin : node->GetInputPins())
			e.searchable_text += " " + pin->GetName();
		for (auto* pin : node->GetOutputPins())
			e.searchable_text += " " + pin->GetName();
		m_index.push_back(e);
	}
}

static String ToLower(CONST String& s)
{
	String r = s;
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

Vector<UInt64> SearchService::Search(CONST String& query) CONST
{
	Vector<UInt64> results;
	if (query.empty()) return results;
	String q = ToLower(query);
	for (auto& e : m_index)
	{
		String hay = ToLower(e.searchable_text);
		if (hay.find(q) != String::npos)
			results.push_back(e.node_id);
	}
	return results;
}

UInt64 SearchService::GetNextMatch(CONST String& query, UInt64 current_match) CONST
{
	auto matches = Search(query);
	if (matches.empty()) return 0;
	for (UInt32 i = 0; i < matches.size(); ++i)
	{
		if (matches[i] == current_match)
			return matches[(i + 1) % matches.size()];
	}
	return matches[0];
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
