#pragma once
#ifndef _SAMPLEAPP_
#define _SAMPLEAPP_
#include "Core/ConstDefine.h"
#include "Render/RenderInterface.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
class Texture;
class CommandList;
MYRENDERER_END_NAMESPACE

MYRENDERER_BEGIN_NAMESPACE(Application)
class Window;

// Intermediate base for sample apps. Collapses the boilerplate every sample
// used to copy: the 8-line main(), BackBuffer/DepthStencil retained-resource
// registration, the SetRenderTarget+clear-value block, and the RDG-to-JSON
// serialization. Subclasses implement OnInitScene() (add passes on `graph`)
// and optionally OnShutdownScene()/OnUpdate()/OnRender().
MYRENDERER_BEGIN_CLASS_WITH_DERIVE(SampleApp, public MXRender::RenderInterface)

#pragma region METHOD
public:
	SampleApp() MYDEFAULT;
	VIRTUAL ~SampleApp() MYDEFAULT;

	// The whole main() of a sample:
	//   int main() { MyApp app; return Application::SampleApp::RunSample(app, "My Sample"); }
	static Int METHOD(RunSample)(SampleApp& in_app, CONST String& in_title = "MXRender");

	// ==== RenderInterface (template methods) ====
	// window cache -> register BackBuffer/DepthStencil retained resources ->
	// OnInitScene() -> graph.Compile() (when auto_compile)
	VIRTUAL void METHOD(OnInit_Logic)(Application::Window* in_window) OVERRIDE FINAL;
	VIRTUAL void METHOD(OnShutdown_Logic)() OVERRIDE FINAL;
	VIRTUAL void METHOD(OnRender)() OVERRIDE;

	// ==== Subclass hooks ====
	VIRTUAL void METHOD(OnInitScene)() PURE;
	VIRTUAL void METHOD(OnShutdownScene)() {}

	// ==== Boilerplate helpers ====
	Application::Window* METHOD(GetWindow)() CONST { return window; }
	RHI::Texture* METHOD(GetBackBuffer)() CONST;
	RHI::Texture* METHOD(GetDepthStencil)() CONST;
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* METHOD(GetBackBufferResource)() CONST { return backbuffer_resource; }
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* METHOD(GetDepthStencilResource)() CONST { return depth_stencil_resource; }
	// SetRenderTarget on the backbuffer (+DSV) with per-target clear values -
	// call inside a pass execute lambda.
	void METHOD(BindBackBufferTarget)(RHI::CommandList* in_cmd) CONST;
	// Serialize the current graph to a .rgraph.json the editor can open.
	// Not called automatically - invoke from OnShutdownScene when wanted.
	void METHOD(SaveGraphDefinition)(CONST String& in_graph_name, CONST String& in_file_path);
protected:

private:

#pragma endregion

#pragma region MEMBER
public:

protected:
	Application::Window* window = nullptr;
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* backbuffer_resource = nullptr;
	Render::RenderGraphResource<RHI::TextureDesc, RHI::Texture>* depth_stencil_resource = nullptr;
	Bool auto_compile = true;
private:

#pragma endregion

MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
