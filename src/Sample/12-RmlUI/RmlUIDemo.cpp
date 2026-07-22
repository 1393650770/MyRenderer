#include "RmlUIDemo.h"

#include "UI/RmlUI/RmlUIManager.h"
#include "UI/RmlUI/RmlUIInputBridge.h"
#include "UI/UIRenderer.h"
#include "UI/UIRenderPass.h"
#include "RHI/RenderViewport.h"
#include "RHI/RenderCommandList.h"
#include "RHI/RenderRHI.h"
#include "Platform/PlatformWindow.h"
#include "Input/InputSystem.h"
#include <iostream>

RmlUIDemoApp::RmlUIDemoApp() {}
RmlUIDemoApp::~RmlUIDemoApp() {}

void RmlUIDemoApp::OnInitScene()
{
	std::cout << "[RmlUIDemo] OnInitScene" << std::endl;

	m_manager = MXRender::UI::RmlUI::RmlUIManager::Get();
	m_manager->Init(viewport);

	m_manager->LoadFontFace("Font/ark-pixel-font-10px-monospaced-ttf-v2026.07.20/ark-pixel-10px-monospaced-latin.ttf");
	m_manager->LoadFontFace("Font/ark-pixel-font-10px-monospaced-ttf-v2026.07.20/ark-pixel-10px-monospaced-zh_cn.ttf");

	m_hud_model = m_manager->CreateDataModel("hud");
	if (m_hud_model.IsValid())
	{
		m_manager->BindVariable(m_hud_model, "hp", &m_hp);
		m_manager->BindVariable(m_hud_model, "score", &m_score);
		m_manager->BindVariable(m_hud_model, "timer", &m_timer);

		m_manager->BindEventCallback(m_hud_model, "on_heal",
			[this](Rml::DataModelHandle handle, Rml::Event& event, const Rml::VariantList& arguments) {
				m_hp = (m_hp + 10);
				if (m_hp > 100) m_hp = 100;
				handle.DirtyVariable("hp");
				std::cout << "[RmlUIDemo] Heal clicked! HP now: " << m_hp << std::endl;
			});
	}

	m_hud_doc = m_manager->LoadDocument("RmlUI/DemoPanel.rml");
	if (m_hud_doc.IsValid())
	{
		m_manager->ShowDocument(m_hud_doc);
		std::cout << "[RmlUIDemo] Document loaded and shown." << std::endl;
	}
	else
	{
		std::cerr << "[RmlUIDemo] Failed to load DemoPanel.rml!" << std::endl;
	}

	auto size = viewport->GetViewportSize();
	MXRender::UI::RegisterUIPass(
		&graph,
		GetBackBufferResource(),
		m_manager->GetRenderer(),
		RHIGetImmediateCommandList(),
		size[0], size[1],
		[this](MXRender::RHI::CommandList* cmd) {
			BindBackBufferTarget(cmd);
			m_manager->Render(cmd);
		});

	for (auto& pass : graph.GetPasses())
		pass->SetIsCullable(false);
}

void RmlUIDemoApp::OnShutdownScene()
{
	std::cout << "[RmlUIDemo] OnShutdownScene" << std::endl;
	if (m_manager)
	{
		m_manager->Shutdown();
		m_manager = nullptr;
	}
}

void RmlUIDemoApp::OnUpdate(float dt)
{
	m_timer += dt;
	m_score = static_cast<int>(m_timer * 10.0f) % 1000;
	m_hp = (m_hp - 1 + 100) % 100;

	if (m_hud_model.IsValid())
	{
		m_manager->DirtyVariable(m_hud_model, "hp");
		m_manager->DirtyVariable(m_hud_model, "score");
		m_manager->DirtyVariable(m_hud_model, "timer");
	}

	// --- Input polling via PlatformWindow + InputSystem (no GLFW) ---
	if (!m_manager) return;
	auto* bridge = m_manager->GetInputBridge();
	auto* pw = GetPlatformWindow();
	if (!bridge || !pw) return;
	auto& input = MXRender::Input::InputSystem::Get();

	// Mouse position
	Float64 mx, my;
	pw->GetCursorPos(mx, my);
	bridge->ProcessMouseMove(static_cast<Int>(mx), static_cast<Int>(my));

	// Mouse buttons
	using MXRender::MouseButton;
	bridge->ProcessMouseButton(0, pw->GetMouseButton(MouseButton::Left));
	bridge->ProcessMouseButton(1, pw->GetMouseButton(MouseButton::Right));
	bridge->ProcessMouseButton(2, pw->GetMouseButton(MouseButton::Middle));

	// Scroll
	Float32 scroll = input.GetScrollDelta();
	if (scroll != 0.0f)
		bridge->ProcessMouseScroll(0.0f, scroll);

	// Keyboard — poll common game UI keys via InputSystem
	using namespace MXRender::Input;
	auto CheckKey = [&](const Key& k) {
		if (input.IsKeyPressed(k))
			bridge->ProcessKey(k, true);
		if (input.IsKeyReleased(k))
			bridge->ProcessKey(k, false);
	};
	CheckKey(EKey::Escape);
	CheckKey(EKey::Tab);
	CheckKey(EKey::Enter);
	CheckKey(EKey::Space);
	CheckKey(EKey::Up);
	CheckKey(EKey::Down);
	CheckKey(EKey::Left);
	CheckKey(EKey::Right);

	m_manager->Update(dt);
}

int main()
{
	RmlUIDemoApp app;
	return MXRender::Application::SampleApp::RunSample(app, "Sample 12 - RmlUI Game UI Demo");
}
