#pragma once
#ifndef _RENDERGRAPH_GRAPHVALIDATOR_
#define _RENDERGRAPH_GRAPHVALIDATOR_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;
class BasePin;

MYRENDERER_BEGIN_STRUCT(ValidationError)
public:
	String message;
	UInt64 node_id = 0;
	UInt64 pin_id = 0;
	String node_name;
	String pin_name;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ValidationWarning)
public:
	String message;
	UInt64 node_id = 0;
	UInt64 pin_id = 0;
	String node_name;
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_STRUCT(ValidationResult)
public:
	Bool is_valid = true;
	Vector<ValidationError> errors;
	Vector<ValidationWarning> warnings;

	void Merge(CONST ValidationResult& other);
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(GraphValidator)

#pragma region METHOD
public:
	GraphValidator() MYDEFAULT;
	~GraphValidator() MYDEFAULT;

	// Validate a graph definition before compilation.
	// Checks: cycle detection (Kahn BFS), pin connectivity, bipartite rules,
	// naming conflicts, orphaned outputs.
	static ValidationResult METHOD(Validate)(CONST Render::RenderGraphDefinition& def);

	// Validate editor-side node/link graph (used before BuildDefinition).
	static ValidationResult METHOD(ValidateEditorGraph)(
		CONST Vector<BaseNode*>& nodes,
		CONST Vector<class BaseLink*>& links);

private:
	static Bool METHOD(DetectCycles)(
		CONST Render::RenderGraphDefinition& def,
		ValidationResult& out_result);

	static void METHOD(CheckPinConnectivity)(
		CONST Render::RenderGraphDefinition& def,
		ValidationResult& out_result);

	static void METHOD(CheckBipartiteRule)(
		CONST Render::RenderGraphDefinition& def,
		ValidationResult& out_result);

	static void METHOD(CheckNamingConflicts)(
		CONST Render::RenderGraphDefinition& def,
		ValidationResult& out_result);

	static void METHOD(CheckOrphanedOutputs)(
		CONST Render::RenderGraphDefinition& def,
		ValidationResult& out_result);

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPH_GRAPHVALIDATOR_
