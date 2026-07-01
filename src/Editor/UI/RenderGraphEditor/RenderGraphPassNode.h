#pragma once
#ifndef _RENDERGRAPHPASSNODE_
#define _RENDERGRAPHPASSNODE_

#include "Core/ConstDefine.h"
#include "UI/BaseNode.h"
#include "RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)
class RenderGraphPassBase;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(UI)

// Specialized node representing a RenderGraph Pass.
// Used for both editing (no runtime pass bound) and visualization (bound to a real pass).
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(RenderGraphPassNode, public BaseNode)

#pragma region METHOD
public:
	VIRTUAL ~RenderGraphPassNode() MYDEFAULT;
	RenderGraphPassNode() MYDEFAULT;
	RenderGraphPassNode(CONST String& in_name, PassNodeType in_pass_type = PassNodeType::Custom, Bool in_show = true);
	RenderGraphPassNode(CONST RenderGraphPassNode& other) MYDELETE;
	RenderGraphPassNode(RenderGraphPassNode&& other) MYDELETE;
	RenderGraphPassNode& operator=(CONST RenderGraphPassNode& other) MYDELETE;
	RenderGraphPassNode& operator=(RenderGraphPassNode&& other) MYDELETE;

	VIRTUAL void METHOD(Draw)() OVERRIDE;
	VIRTUAL void METHOD(Release)() OVERRIDE;
	VIRTUAL BaseNode* METHOD(AsNode)() OVERRIDE { return this; }

	// Pin management with access semantics
	BasePin* METHOD(AddInputPin)(CONST String& in_name, PinAccess access = PinAccess::Read);
	BasePin* METHOD(AddOutputPin)(CONST String& in_name, PinAccess access = PinAccess::Write);

	// Runtime binding
	void METHOD(BindPass)(Render::RenderGraphPassBase* pass);
	Render::RenderGraphPassBase* METHOD(GetBoundPass)() CONST { return bound_pass; }

	// State
	PassNodeType METHOD(GetPassType)() CONST { return pass_type; }
	void METHOD(SetPassType)(PassNodeType type) { pass_type = type; }
	Int METHOD(GetExecutionOrder)() CONST { return execution_order; }
	void METHOD(SetExecutionOrder)(Int order) { execution_order = order; }
	Bool METHOD(GetIsCulled)() CONST { return is_culled; }
	void METHOD(SetIsCulled)(Bool culled) { is_culled = culled; }
	Bool METHOD(GetIsActive)() CONST { return is_active; }
	void METHOD(SetIsActive)(Bool active) { is_active = active; }

protected:
	VIRTUAL void METHOD(RecalcSize)() OVERRIDE;
private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	PassNodeType pass_type = PassNodeType::Custom;
	Render::RenderGraphPassBase* bound_pass = nullptr; // Weak reference to runtime pass
	Int execution_order = -1;
	Bool is_culled = false;
	Bool is_active = false;
private:
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHPASSNODE_
