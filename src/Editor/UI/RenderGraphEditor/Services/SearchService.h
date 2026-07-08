#pragma once
#ifndef _RENDERGRAPH_SEARCHSERVICE_
#define _RENDERGRAPH_SEARCHSERVICE_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;

// --   Search index for node graph filtering
MYRENDERER_BEGIN_STRUCT(SearchEntry)
public:
	UInt64 node_id = 0;
	String searchable_text;
	Int node_type_hint = 0; // PassNodeType or ResourceNodeType int cast
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(SearchService)
public:
	SearchService() MYDEFAULT;
	~SearchService() MYDEFAULT;

	// --   Rebuild search index from node list
	void METHOD(RebuildIndex)(CONST Vector<BaseNode*>& nodes);

	// --   Search with case-insensitive substring matching
	Vector<UInt64> METHOD(Search)(CONST String& query) CONST;

	// --   Get next match after current
	UInt64 METHOD(GetNextMatch)(CONST String& query, UInt64 current_match) CONST;

private:
	Vector<SearchEntry> m_index;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
