#pragma once
#ifndef _RENDERGRAPHSERIALIZER_
#define _RENDERGRAPHSERIALIZER_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// JSON-based serializer for RenderGraphDefinition.
// Uses nlohmann_json (already a project dependency via xmake).
// Saves human-readable .rgraph.json files that can be manually edited.
MYRENDERER_BEGIN_CLASS(RenderGraphSerializer)

#pragma region METHOD
public:
	// Save a graph definition to a JSON file.
	static Bool METHOD(SaveGraph)(CONST RenderGraphDefinition& def, CONST String& filepath);

	// Load a graph definition from a JSON file.
	static Bool METHOD(LoadGraph)(RenderGraphDefinition& out_def, CONST String& filepath);


	// Get last error message.
	static String METHOD(GetLastError)() { return s_last_error; }

private:
	static String s_last_error;

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHSERIALIZER_
