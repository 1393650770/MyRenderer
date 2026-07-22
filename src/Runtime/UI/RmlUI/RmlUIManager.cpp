#include "RmlUIManager.h"
#include "RmlUISystemInterface.h"
#include "RmlUIFileInterface.h"
#include "RmlUIInputBridge.h"
#include "RmlUIRenderer.h"
#include "RmlUIRenderInterface.h"
#include "UI/UIRenderer.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Debugger.h>

#include "RHI/RenderViewport.h"
#include "RHI/RenderRHI.h"
#include "RHI/RenderCommandList.h"
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

RmlUIManager* RmlUIManager::s_instance = nullptr;

RmlUIManager* RmlUIManager::Get()
{
	if (!s_instance)
	{
		s_instance = new RmlUIManager();
	}
	return s_instance;
}

void RmlUIManager::Init(RHI::Viewport* viewport)
{
	if (m_initialized) return;
	m_viewport = viewport;

	// 1. Install engine interfaces
	SetupInterfaces();

	// 2. Initialize RmlUI core
	auto size = viewport->GetViewportSize();
	Rml::Initialise();

	// 3. Create the main context
	Rml::Vector2i dims(static_cast<int>(size[0]), static_cast<int>(size[1]));
	m_context = Rml::CreateContext("main", dims);

	if (!m_context)
	{
		std::cerr << "[RmlUIManager] Failed to create RmlUI context!" << std::endl;
		return;
	}

	std::cout << "[RmlUIManager] Initialized — context \"" << m_context->GetName()
		<< "\" " << dims.x << "x" << dims.y << std::endl;

	// 4. Create the input bridge and wire it to this context
	m_input_bridge = new RmlUIInputBridge();
	m_input_bridge->SetContext(m_context);

	// 5. Create the Vulkan renderer and Rml::RenderInterface adapter
	m_renderer = new RmlUIRenderer();

	auto* rml_ri = new RmlUIRenderInterface(m_renderer);
	Rml::SetRenderInterface(static_cast<Rml::RenderInterface*>(rml_ri->GetRmlInterface()));

	// 6. Initialize renderer GPU resources (shaders, PSOs)
	auto* cmd = RHIGetImmediateCommandList();
	ENUM_TEXTURE_FORMAT rt_format = viewport->GetCurrentBackBufferRTV()->GetTextureDesc().format;
	static_cast<RmlUIRenderer*>(m_renderer)->Initialize(cmd, rt_format);

	m_initialized = true;
}

void RmlUIManager::Shutdown()
{
	if (!m_initialized) return;

	m_context = nullptr; // Rml::Shutdown destroys all contexts

	Rml::Shutdown();
	TeardownInterfaces();

	delete m_input_bridge;
	m_input_bridge = nullptr;

	delete m_renderer;
	m_renderer = nullptr;

	m_initialized = false;
	std::cout << "[RmlUIManager] Shutdown complete." << std::endl;
}

void RmlUIManager::ProcessInput()
{
	// Input is processed by RmlUIInputBridge externally.
	// This method is a placeholder for future batched input processing.
}

void RmlUIManager::Update(Float32 dt)
{
	if (!m_context) return;

	// Update viewport dimensions
	auto size = m_viewport->GetViewportSize();
	m_context->SetDimensions(Rml::Vector2i(static_cast<int>(size[0]), static_cast<int>(size[1])));

	// Process RmlUI layout, animations, data bindings
	m_context->Update();
}

void RmlUIManager::Render(RHI::CommandList* cmd)
{
	if (!m_context || !m_renderer) return;

	m_renderer->BeginFrame(cmd);
	m_context->Render();
	m_renderer->EndFrame(cmd);
}

Rml::Context* RmlUIManager::GetContext() CONST
{
	return m_context;
}

UI::UIRenderer* RmlUIManager::GetRenderer() CONST
{
	return m_renderer;
}

RmlUIInputBridge* RmlUIManager::GetInputBridge() CONST
{
	return m_input_bridge;
}

bool RmlUIManager::LoadFontFace(CONST String& file_path)
{
	if (!m_initialized)
		return false;
	// Rml::LoadFontFace is a global function
	return Rml::LoadFontFace(file_path);
}

void RmlUIManager::SetupInterfaces()
{
	m_system_interface = new RmlUISystemInterface();
	m_system_interface->Install();

	m_file_interface = new RmlUIFileInterface();
	m_file_interface->Install();
}

void RmlUIManager::TeardownInterfaces()
{
	m_system_interface->Uninstall();
	delete m_system_interface;
	m_system_interface = nullptr;

	m_file_interface->Uninstall();
	delete m_file_interface;
	m_file_interface = nullptr;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
