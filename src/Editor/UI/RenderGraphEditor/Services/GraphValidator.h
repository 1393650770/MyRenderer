#pragma once
#ifndef _RENDERGRAPH_GRAPHVALIDATOR_
#define _RENDERGRAPH_GRAPHVALIDATOR_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphValidator.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BasePin;

// Editor-side graph validation. Runtime-level validation is in Render::RenderGraphValidator.
// This class adds editor-specific checks (e.g., duplicate node names by editor graph node).
MYRENDERER_BEGIN_CLASS(GraphValidator)

#pragma region METHOD
public:
	GraphValidator() MYDEFAULT;
	~GraphValidator() MYDEFAULT;

	// Validate a graph definition (delegates to Runtime validator).
	static Render::ValidationResult METHOD(Validate)(CONST Render::RenderGraphDefinition& def)
	{
		return Render::RenderGraphValidator::Validate(def);
	}

	// Validate editor-side node/link graph (used before BuildDefinition).
	static Render::ValidationResult METHOD(ValidateEditorGraph)(
		CONST Vector<BaseNode*>& nodes,
		CONST Vector<class BaseLink*>& links);

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
