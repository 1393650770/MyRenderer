#include "RmlUIDemo.h"

#include "UI/RmlUI/RmlUIManager.h"
#include "UI/RmlUI/RmlUIInputBridge.h"
#include "UI/UIRenderer.h"
#include "UI/UIRenderPass.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderRHI.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

#include <iostream>

// =========================================================================
// RmlUIDemoApp — Sample 12
// =========================================================================

RmlUIDemoApp::RmlUIDemoApp()
{
}

RmlUIDemoApp::~RmlUIDemoApp()
{
}

void RmlUIDemoApp::OnInitScene()
{
	std::cout << "[RmlUIDemo] OnInitScene" << std::endl;

	// 1. Get or create the RmlUI manager singleton
	m_manager = MXRender::UI::RmlUI::RmlUIManager::Get();

	// 2. Initialize RmlUI with the engine viewport
	m_manager->Init(viewport);

	// 3. Load a font (required before rendering text)
	m_manager->LoadFontFace("Editor/Roboto-Regular.ttf");

	// 4. Create data model binding (manual binding without code-gen for the demo)
	auto* ctx = m_manager->GetContext();
	if (ctx)
	{
		auto ctor = ctx->CreateDataModel("hud");
		if (ctor)
		{
			// Bind data variables
			ctor.Bind("hp", &m_hp);
			ctor.Bind("score", &m_score);
			ctor.Bind("timer", &m_timer);

			// Bind event callback
			ctor.BindEventCallback("on_heal", [this](Rml::DataModelHandle handle, Rml::Event& event, const Rml::VariantList& arguments) {
				m_hp = (m_hp + 10);
				if (m_hp > 100) m_hp = 100;
				handle.DirtyVariable("hp");
				std::cout << "[RmlUIDemo] Heal clicked! HP now: " << m_hp << std::endl;
			});

			m_hud_handle = new Rml::DataModelHandle(ctor.GetModelHandle());
		}

		// 5. Load the demo RML document
		auto* doc = ctx->LoadDocument("RmlUI/DemoPanel.rml");
		if (doc)
		{
			doc->Show();
			std::cout << "[RmlUIDemo] Document loaded and shown." << std::endl;
		}
		else
		{
			std::cerr << "[RmlUIDemo] Failed to load DemoPanel.rml!" << std::endl;
		}
	}

	// 6. Register the RmlUI RDG pass
	auto size = viewport->GetViewportSize();
	MXRender::UI::RegisterUIPass(
		&graph,
		GetBackBufferResource(),
		m_manager->GetRenderer(),
		RHIGetImmediateCommandList(),
		size[0], size[1]);

	// 7. Set cullable=false for all passes (they have no creator pass edges to keep them alive)
	for (auto& pass : graph.GetPasses())
	{
		// All RDG passes that only read/write retained resources (no transient creates)
		// need SetIsCullable(false) so they aren't dropped from the timeline.
		pass->SetIsCullable(false);
	}
}

void RmlUIDemoApp::OnShutdownScene()
{
	std::cout << "[RmlUIDemo] OnShutdownScene" << std::endl;
	delete m_hud_handle;
	m_hud_handle = nullptr;

	if (m_manager)
	{
		m_manager->Shutdown();
		m_manager = nullptr;
	}
}

void RmlUIDemoApp::OnUpdate(float dt)
{
	// Update demo data
	m_timer += dt;
	m_score = static_cast<int>(m_timer * 10.0f) % 1000;

	// Simulate HP decreasing
	m_hp = (m_hp - 1 + 100) % 100;

	// Notify RmlUI about data changes
	if (m_hud_handle)
	{
		m_hud_handle->DirtyVariable("hp");
		m_hud_handle->DirtyVariable("score");
		m_hud_handle->DirtyVariable("timer");
	}

	// Process input and update RmlUI context
	auto* bridge = m_manager->GetInputBridge();

	// Poll GLFW for input (direct GLFW access — Sample pattern)
	auto* glfw_ctx = glfwGetCurrentContext();
	if (glfw_ctx)
	{
		double mx, my;
		glfwGetCursorPos(glfw_ctx, &mx, &my);
		bridge->ProcessMouseMove(static_cast<Int32>(mx), static_cast<Int32>(my));

		for (int btn = 0; btn < 3; btn++)
		{
			int state = glfwGetMouseButton(glfw_ctx, GLFW_MOUSE_BUTTON_1 + btn);
			bridge->ProcessMouseButton(btn, state == GLFW_PRESS);
		}
	}

	m_manager->Update(dt);
}

// =========================================================================
// main()
// =========================================================================
int main()
{
	RmlUIDemoApp app;
	return MXRender::Application::SampleApp::RunSample(app, "Sample 12 - RmlUI Game UI Demo");
}
