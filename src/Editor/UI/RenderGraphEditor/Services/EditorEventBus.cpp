// --   --
#include "UI/RenderGraphEditor/Services/EditorEventBus.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

EditorEventBus& EditorEventBus::Get()
{
	static EditorEventBus instance;
	return instance;
}

void EditorEventBus::AddListener(IEditorListener* listener)
{
	if (listener) listeners.push_back(listener);
}

void EditorEventBus::RemoveListener(IEditorListener* listener)
{
	auto it = std::find(listeners.begin(), listeners.end(), listener);
	if (it != listeners.end()) listeners.erase(it);
}

void EditorEventBus::FireNodeRenamed(UInt64 node_id, CONST String& old_name, CONST String& new_name)
{
	auto copy = listeners;
	for (auto* l : copy) l->OnNodeRenamed(node_id, old_name, new_name);
}

void EditorEventBus::FirePropertyChanged(UInt64 node_id, CONST String& prop_name)
{
	auto copy = listeners;
	for (auto* l : copy) l->OnPropertyChanged(node_id, prop_name);
}

void EditorEventBus::FireSelectionChanged(BaseNode* new_selection)
{
	auto copy = listeners;
	for (auto* l : copy) l->OnSelectionChanged(new_selection);
}

void EditorEventBus::FireGraphModified()
{
	pending_graph_modified = true;
}

void EditorEventBus::TickFireGraphModified()
{
	if (pending_graph_modified)
	{
		frame_counter++;
		if (frame_counter >= DEBOUNCE_FRAMES)
		{
			auto copy = listeners;
			for (auto* l : copy) l->OnGraphModified();
			pending_graph_modified = false;
			frame_counter = 0;
		}
	}
}

void EditorEventBus::FireNodeDeleted(UInt64 node_id)
{
	auto copy = listeners;
	for (auto* l : copy) l->OnNodeDeleted(node_id);
}

void EditorEventBus::FireNodeAdded(UInt64 node_id)
{
	auto copy = listeners;
	for (auto* l : copy) l->OnNodeAdded(node_id);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
