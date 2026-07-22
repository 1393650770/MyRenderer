#pragma once
#ifndef _RMLUIMANAGER_
#define _RMLUIMANAGER_

#include "Core/ConstDefine.h"
#include "UI/UIBase.h"
#include "UI/UIHandleTypes.h"

#include <RmlUi/Core/DataModelHandle.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
class CommandList;
MYRENDERER_END_NAMESPACE
MYRENDERER_BEGIN_NAMESPACE(Render)
class RenderGraph;
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
class UIRenderer;
MYRENDERER_BEGIN_NAMESPACE(RmlUI)
class RmlUISystemInterface;
class RmlUIFileInterface;
class RmlUIInputBridge;
class RmlUIRenderInterface;

/**
 * RmlUI Manager — singleton that owns the RmlUI lifecycle.
 *
 * All RmlUI types are encapsulated behind this class.
 * Upper-layer code (Sample, Editor) never includes <RmlUi/...> directly.
 *
 * Pattern:
 *   auto* mgr = RmlUIManager::Get();
 *   mgr->Init(viewport);
 *   auto hud = mgr->CreateDataModel("hud");
 *   mgr->BindVariable(hud, "hp", &player.hp);
 *   mgr->BindEventCallback(hud, "on_click", my_callback);
 *   auto doc = mgr->LoadDocument("RmlUI/panel.rml");
 *   mgr->ShowDocument(doc);
 *   // each frame:
 *   mgr->DirtyVariable(hud, "hp");
 *   mgr->Update(dt);
 */
MYRENDERER_BEGIN_CLASS(RmlUIManager)

#pragma region METHOD
public:
	RmlUIManager() MYDEFAULT;
	VIRTUAL ~RmlUIManager();

	/// Get the global singleton.
	static RmlUIManager* METHOD(Get)();

	/// Initialize RmlUI, create context and renderer.
	void METHOD(Init)(RHI::Viewport* viewport);

	/// Shutdown RmlUI, release all resources.
	void METHOD(Shutdown)();

	/// Process accumulated input events for this frame.
	void METHOD(ProcessInput)();

	/// Update RmlUI context and process animations/layout.
	void METHOD(Update)(Float32 dt);

	/// Render the UI. Called inside an RDG pass.
	void METHOD(Render)(RHI::CommandList* cmd);

	// -----------------------------------------------------------------------
	// Data model binding (type-erased API)
	// -----------------------------------------------------------------------

	/// Create a named data model. Returns opaque handle.
	RmlModelHandle METHOD(CreateDataModel)(CONST String& name, bool allow_missing = false);

	/// Bind a C++ variable to a data model. Call after CreateDataModel.
	/// @tparam T  Variable type (int, float, String, etc.)
	template<typename T>
	void METHOD(BindVariable)(RmlModelHandle model, CONST String& name, T* ptr)
	{
		auto* ctor = GetModelConstructor(model);
		if (ctor) ctor->Bind(name, ptr);
	}

	/// Bind an event callback to a data model.
	void METHOD(BindEventCallback)(RmlModelHandle model, CONST String& name,
		Rml::DataEventFunc func);

	/// Notify RmlUI that a bound variable has changed (triggers UI refresh).
	void METHOD(DirtyVariable)(RmlModelHandle model, CONST String& name);

	/// Remove a data model.
	void METHOD(RemoveDataModel)(RmlModelHandle model);

	// -----------------------------------------------------------------------
	// Document management (type-erased API)
	// -----------------------------------------------------------------------

	/// Load an .rml document and return opaque handle.
	RmlDocHandle METHOD(LoadDocument)(CONST String& path);

	/// Show a loaded document.
	void METHOD(ShowDocument)(RmlDocHandle doc);

	/// Hide a document.
	void METHOD(HideDocument)(RmlDocHandle doc);

	/// Close and unload a document.
	void METHOD(CloseDocument)(RmlDocHandle doc);

	// -----------------------------------------------------------------------
	// Accessors
	// -----------------------------------------------------------------------

	/// Get the renderer backend.
	UI::UIRenderer* METHOD(GetRenderer)() CONST;

	/// Get the input bridge (for priority config).
	RmlUIInputBridge* METHOD(GetInputBridge)() CONST;

	/// Load a font face from file.
	bool METHOD(LoadFontFace)(CONST String& file_path);

	/// Returns true if the UI is capturing mouse input.
	bool METHOD(IsMouseInteracting)() CONST;

protected:
private:
	void METHOD(SetupInterfaces)();
	void METHOD(TeardownInterfaces)();

	/// Internal: get constructor for a model handle. Returns nullptr if invalid.
	Rml::DataModelConstructor* METHOD(GetModelConstructor)(RmlModelHandle model);
#pragma endregion

#pragma region MEMBER
public:
protected:
private:
	static RmlUIManager* s_instance;

	// Owned interfaces
	RmlUISystemInterface* m_system_interface = nullptr;
	RmlUIFileInterface*   m_file_interface = nullptr;
	RmlUIInputBridge*     m_input_bridge = nullptr;

	// RmlUI context (internal, not exposed to upper layers)
	void* m_context = nullptr; // Rml::Context*, type-erased to avoid header dependency

	// Renderer
	UI::UIRenderer* m_renderer = nullptr;
	RmlUIRenderInterface* m_render_interface = nullptr;

	// Viewport reference
	RHI::Viewport* m_viewport = nullptr;

	bool m_initialized = false;

	// Internal data model registry (handle → constructor)
	void* m_model_registry = nullptr; // Map<GenericHandle, ModelEntry>*, allocated in .cpp

	// Internal document registry (handle → ElementDocument*)
	void* m_doc_registry = nullptr;

	// Next handles
	UInt32 m_next_model_handle = 1;
	UInt32 m_next_doc_handle = 1;
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_RMLUIMANAGER_
