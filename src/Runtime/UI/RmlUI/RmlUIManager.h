#pragma once
#ifndef _RMLUIMANAGER_
#define _RMLUIMANAGER_

#include "Core/ConstDefine.h"
#include "UI/UIBase.h"

// Forward declarations
namespace Rml {
class Context;
class DataModelHandle;
}
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Viewport;
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

/**
 * RmlUI Manager — singleton that owns the RmlUI lifecycle.
 *
 * Pattern: Init once at app start, Update+Rerender each frame, Shutdown at app end.
 * Renders through a UIRenderer backend (created by Init).
 *
 * Usage:
 *   auto* mgr = RmlUIManager::Get();
 *   mgr->Init(viewport, graph, bb_resource);
 *   // ... load documents, create data models ...
 *   // each frame:
 *   mgr->ProcessInput();  // in OnUpdate
 *   mgr->Update(dt);      // in OnUpdate after input
 *   // rendering happens in RDG pass
 *
 * Thread safety: All methods must be called from the same thread.
 */
MYRENDERER_BEGIN_CLASS(RmlUIManager)

#pragma region METHOD
public:
	RmlUIManager() MYDEFAULT;
	VIRTUAL ~RmlUIManager() MYDEFAULT;

	/// Get the global singleton.
	static RmlUIManager* METHOD(Get)();

	/// Initialize RmlUI, create context and renderer.
	/// @param viewport   Engine viewport (for dimensions)
	void METHOD(Init)(RHI::Viewport* viewport);

	/// Shutdown RmlUI, release all resources.
	void METHOD(Shutdown)();

	/// Process accumulated input events for this frame.
	/// Call after glfwPollEvents() in the main loop.
	void METHOD(ProcessInput)();

	/// Update RmlUI context and process animations/layout.
	/// Call after ProcessInput() in the main loop.
	void METHOD(Update)(Float32 dt);

	/// Render the UI. Called inside an RDG pass.
	/// @param cmd  Active RHI command list (from RDG pass execute lambda)
	void METHOD(Render)(RHI::CommandList* cmd);

	/// Get the RmlUI context (for creating data models, loading documents, etc.)
	Rml::Context* METHOD(GetContext)() CONST;

	/// Get the renderer backend.
	UI::UIRenderer* METHOD(GetRenderer)() CONST;

	/// Set the input bridge (created by Init, accessible for priority config).
	RmlUIInputBridge* METHOD(GetInputBridge)() CONST;

	/// Load a font face from file (must be called after Init, before creating documents).
	bool METHOD(LoadFontFace)(CONST String& file_path);

protected:
private:
	void METHOD(SetupInterfaces)();
	void METHOD(TeardownInterfaces)();
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

	// RmlUI objects
	Rml::Context* m_context = nullptr;

	// Renderer (created by Init based on RHI backend)
	UI::UIRenderer* m_renderer = nullptr;

	// Viewport reference (for dimensions, not owned)
	RHI::Viewport* m_viewport = nullptr;

	bool m_initialized = false;
#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_RMLUIMANAGER_
