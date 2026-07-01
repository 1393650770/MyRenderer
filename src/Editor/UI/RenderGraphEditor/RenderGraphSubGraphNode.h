#pragma once
#ifndef _RENDERGRAPHSUBGRAPHNODE_
#define _RENDERGRAPHSUBGRAPHNODE_

#include "Core/ConstDefine.h"
#include "UI/BaseNode.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class RenderGraphPassNode;
class RenderGraphResourceNode;

// Collapsed: shows header + exposed pins (1:1 mapped to child pins).
// Expanded: renders child nodes and internal connections inline.
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphSubGraphNode, public BaseNode)

#pragma region METHOD
public:
	VIRTUAL ~RenderGraphSubGraphNode() MYDEFAULT;
	RenderGraphSubGraphNode() MYDEFAULT;
	RenderGraphSubGraphNode(CONST String& in_name, Bool in_show = true);

	VIRTUAL void METHOD(Draw)() OVERRIDE;
	VIRTUAL void METHOD(Release)() OVERRIDE;
	VIRTUAL BaseNode* METHOD(AsNode)() OVERRIDE { return this; }

	void METHOD(AddChildPass)(RenderGraphPassNode* pass);
	void METHOD(AddChildResource)(RenderGraphResourceNode* resource);
	void METHOD(RemoveChild)(UInt64 node_id);
	Vector<BaseNode*>& METHOD(GetChildren)() { return m_children; }

	// Uses BaseNode::AddInput/AddOutput to create pins on the subgraph node itself.
	void METHOD(ExposeInput)(CONST String& exposed_name, BaseNode* child, CONST String& child_pin_name);
	void METHOD(ExposeOutput)(CONST String& exposed_name, BaseNode* child, CONST String& child_pin_name);

	Bool METHOD(GetIsCollapsed)() CONST { return m_collapsed; }
	void METHOD(SetCollapsed)(Bool collapsed);
	void METHOD(ToggleCollapsed)() { SetCollapsed(!m_collapsed); }

	static Bool METHOD(IsSubGraphNode)(BaseNode* node);

	ImU32 METHOD(GetColor)() CONST { return m_color; }
	void METHOD(SetColor)(ImU32 color) { m_color = color; }

protected:
	VIRTUAL void METHOD(RecalcSize)() OVERRIDE;
private:
	void DrawCollapsed();
	void DrawExpandedHeader();

#pragma endregion

#pragma region MEMBER
public:
protected:
	Vector<BaseNode*> m_children;
	Vector<std::pair<UInt64, String>> m_exposed_inputs;
	Vector<std::pair<UInt64, String>> m_exposed_outputs;
	Bool m_collapsed = true;
	ImU32 m_color = IM_COL32(80, 80, 128, 60); // Default subgraph tint
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHSUBGRAPHNODE_
