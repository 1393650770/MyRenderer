#pragma once
#ifndef _RENDERGRAPH_EDITOREVENTBUS_
#define _RENDERGRAPH_EDITOREVENTBUS_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

class BaseNode;

// -- [AI:BEGIN] --
// Observer interface for editor panel communication.
// Single-threaded: all calls must be from the ImGui main thread.
// -- [AI:END] --
MYRENDERER_BEGIN_CLASS(IEditorListener)
public:
	VIRTUAL ~IEditorListener() MYDEFAULT;
	VIRTUAL void METHOD(OnNodeRenamed)(UInt64 node_id, CONST String& old_name, CONST String& new_name) {}
	VIRTUAL void METHOD(OnPropertyChanged)(UInt64 node_id, CONST String& prop_name) {}
	VIRTUAL void METHOD(OnSelectionChanged)(BaseNode* new_selection) {}
	VIRTUAL void METHOD(OnGraphModified)() {}
	VIRTUAL void METHOD(OnNodeDeleted)(UInt64 node_id) {}
	VIRTUAL void METHOD(OnNodeAdded)(UInt64 node_id) {}
MYRENDERER_END_CLASS

MYRENDERER_BEGIN_CLASS(EditorEventBus)
#pragma region METHOD
public:
	static EditorEventBus& METHOD(Get)();

	void METHOD(AddListener)(IEditorListener* listener);
	void METHOD(RemoveListener)(IEditorListener* listener);

	// -- [AI] Fire events. Iterates a COPY of the listener list to prevent
	// iterator invalidation when a listener callback triggers another event.
	void METHOD(FireNodeRenamed)(UInt64 node_id, CONST String& old_name, CONST String& new_name);
	void METHOD(FirePropertyChanged)(UInt64 node_id, CONST String& prop_name);
	void METHOD(FireSelectionChanged)(BaseNode* new_selection);
	void METHOD(FireGraphModified)();
	void METHOD(FireNodeDeleted)(UInt64 node_id);
	void METHOD(FireNodeAdded)(UInt64 node_id);

	// -- [AI] Debounced graph-modified. Call per-frame from Draw().
	// Only actually fires if >= 3 frames since last fire.
	void METHOD(TickFireGraphModified)();
private:
	EditorEventBus() MYDEFAULT;
#pragma endregion

#pragma region MEMBER
protected:
	Vector<IEditorListener*> listeners;
	Bool pending_graph_modified = false;
	UInt32 frame_counter = 0;
	static constexpr UInt32 DEBOUNCE_FRAMES = 3;
private:
#pragma endregion
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPH_EDITOREVENTBUS_
