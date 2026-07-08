#pragma once
#ifndef _RENDERGRAPH_RENDERGRAPHVVALIDATOR_
#define _RENDERGRAPH_RENDERGRAPHVVALIDATOR_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

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

// Runtime-level graph validator. Operates on RenderGraphDefinition.
// Editor-side per-node validation (duplicate names, etc.) lives in UI::GraphValidator.
MYRENDERER_BEGIN_CLASS(RenderGraphValidator)

#pragma region METHOD
public:
	RenderGraphValidator() MYDEFAULT;
	~RenderGraphValidator() MYDEFAULT;

	static ValidationResult METHOD(Validate)(CONST RenderGraphDefinition& def);

private:
	static Bool METHOD(DetectCycles)(CONST RenderGraphDefinition& def, ValidationResult& out_result);
	static void METHOD(CheckPinConnectivity)(CONST RenderGraphDefinition& def, ValidationResult& out_result);
	static void METHOD(CheckBipartiteRule)(CONST RenderGraphDefinition& def, ValidationResult& out_result);
	static void METHOD(CheckNamingConflicts)(CONST RenderGraphDefinition& def, ValidationResult& out_result);
	static void METHOD(CheckOrphanedOutputs)(CONST RenderGraphDefinition& def, ValidationResult& out_result);

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
