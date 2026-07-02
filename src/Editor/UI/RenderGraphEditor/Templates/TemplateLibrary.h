#pragma once
#ifndef _RENDERGRAPH_TEMPLATELIBRARY_
#define _RENDERGRAPH_TEMPLATELIBRARY_

#include "Core/ConstDefine.h"
#include "UI/RenderGraphEditor/Templates/PassTemplate.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// -- [AI] Template library ¡ª built-in and user-defined templates
MYRENDERER_BEGIN_CLASS(TemplateLibrary)
public:
	TemplateLibrary() MYDEFAULT;
	~TemplateLibrary() MYDEFAULT;

	// -- [AI] Get all available templates
	static CONST Vector<PassTemplate>& METHOD(GetBuiltinTemplates)();

	// -- [AI] Find a template by name
	static CONST PassTemplate* METHOD(FindTemplate)(CONST String& name);

	// -- [AI] Register a user-defined template
	static void METHOD(RegisterUserTemplate)(CONST PassTemplate& tmpl);

	// -- [AI] Save/load user templates to .rgtemplate.json
	static void METHOD(LoadUserTemplates)(CONST String& dir_path);

private:
public:
	static Vector<PassTemplate> s_builtin;
	static Vector<PassTemplate> s_user;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
