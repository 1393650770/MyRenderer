#include "UI/RenderGraphEditor/Services/GraphValidator.h"
#include "UI/BaseNode.h"
#include "UI/BasePin.h"
#include "UI/BaseLink.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphPassNode.h"
#include "UI/RenderGraphEditor/Nodes/RenderGraphResourceNode.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

Render::ValidationResult GraphValidator::ValidateEditorGraph(
	CONST Vector<BaseNode*>& nodes,
	CONST Vector<class BaseLink*>& links)
{
	Render::ValidationResult result;
	Set<String> names;
	for (auto* node : nodes) {
		if (!node) continue;
		if (names.count(node->GetName())) {
			Render::ValidationError err; err.message = "Duplicate node: " + node->GetName();
			err.node_id = node->GetSelfID(); err.node_name = node->GetName();
			result.errors.push_back(err); result.is_valid = false;
		}
		names.insert(node->GetName());
	}
	return result;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
