#include "UI/RenderGraphEditor/Templates/TemplateLibrary.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

Vector<PassTemplate> TemplateLibrary::s_builtin;
Vector<PassTemplate> TemplateLibrary::s_user;

static void InitBuiltins()
{
	if (!TemplateLibrary::s_builtin.empty()) return;

	// GBuffer Pass template
	PassTemplate gbuffer;
	gbuffer.name = "GBuffer Pass";
	gbuffer.description = "Renders albedo, normal, roughness, depth to G-Buffer targets";
	gbuffer.category = "Geometry";
	gbuffer.pass_kind = Render::RDGPassKind::Graphics;
	gbuffer.output_pins = {
		{"GBufferA", PinAccess::Write, Render::RDGResourceKind::Texture},
		{"GBufferB", PinAccess::Write, Render::RDGResourceKind::Texture},
		{"GBufferC", PinAccess::Write, Render::RDGResourceKind::Texture},
		{"SceneDepth", PinAccess::Write, Render::RDGResourceKind::DepthStencil},
	};
	{
		Render::RDGResourceDef gba;
		gba.name = "GBufferA"; gba.kind = Render::RDGResourceKind::Texture;
		gba.width = 1920; gba.height = 1080;
		gbuffer.auto_create_resources.push_back(gba);
	}
	{
		Render::RDGResourceDef gbb;
		gbb.name = "GBufferB"; gbb.kind = Render::RDGResourceKind::Texture;
		gbb.width = 1920; gbb.height = 1080;
		gbuffer.auto_create_resources.push_back(gbb);
	}
	{
		Render::RDGResourceDef gbc;
		gbc.name = "GBufferC"; gbc.kind = Render::RDGResourceKind::Texture;
		gbc.width = 1920; gbc.height = 1080;
		gbuffer.auto_create_resources.push_back(gbc);
	}
	TemplateLibrary::s_builtin.push_back(gbuffer);

	// Lighting Pass
	PassTemplate lighting;
	lighting.name = "Lighting Pass";
	lighting.description = "Deferred lighting �� reads GBuffer, outputs light buffer";
	lighting.category = "Lighting";
	lighting.pass_kind = Render::RDGPassKind::Graphics;
	lighting.input_pins = {
		{"GBufferA", PinAccess::Read, Render::RDGResourceKind::Texture},
		{"GBufferB", PinAccess::Read, Render::RDGResourceKind::Texture},
		{"GBufferC", PinAccess::Read, Render::RDGResourceKind::Texture},
		{"SceneDepth", PinAccess::Read, Render::RDGResourceKind::DepthStencil},
	};
	lighting.output_pins = {		{"LightBuffer", PinAccess::Write, Render::RDGResourceKind::Texture},	};
	TemplateLibrary::s_builtin.push_back(lighting);

	// Shadow Pass
	PassTemplate shadow;
	shadow.name = "Shadow Pass";
	shadow.description = "Depth-only shadow map rendering";
	shadow.category = "Geometry";
	shadow.pass_kind = Render::RDGPassKind::Graphics;
	shadow.output_pins = {		{"ShadowMap", PinAccess::Write, Render::RDGResourceKind::DepthStencil},	};
	TemplateLibrary::s_builtin.push_back(shadow);

	// PostProcess Pass
	PassTemplate post;
	post.name = "Post Process Pass";
	post.description = "Fullscreen post-process (bloom, tonemap, etc.)";
	post.category = "PostProcess";
	post.pass_kind = Render::RDGPassKind::Graphics;
	post.input_pins = {		{"SceneColor", PinAccess::Read, Render::RDGResourceKind::Texture},	};
	post.output_pins = {		{"PostOutput", PinAccess::Write, Render::RDGResourceKind::Texture},	};
	TemplateLibrary::s_builtin.push_back(post);
}

CONST Vector<PassTemplate>& TemplateLibrary::GetBuiltinTemplates()
{
	InitBuiltins();
	return s_builtin;
}

CONST PassTemplate* TemplateLibrary::FindTemplate(CONST String& name)
{
	InitBuiltins();
	for (auto& t : s_builtin) if (t.name == name) return &t;
	for (auto& t : s_user) if (t.name == name) return &t;
	return nullptr;
}

void TemplateLibrary::RegisterUserTemplate(CONST PassTemplate& tmpl)
{
	s_user.push_back(tmpl);
}

void TemplateLibrary::LoadUserTemplates(CONST String& dir_path)
{
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
