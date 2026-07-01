#pragma once
#ifndef _RENDERGRAPH_TEMPLATELIBRARY_
#define _RENDERGRAPH_TEMPLATELIBRARY_

#include "Core/ConstDefine.h"
#include "UI/RenderGraphEditor/Templates/PassTemplate.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

MYRENDERER_BEGIN_CLASS(TemplateLibrary)
public:
	TemplateLibrary() MYDEFAULT;
	~TemplateLibrary() MYDEFAULT;

	static CONST Vector<PassTemplate>& METHOD(GetBuiltinTemplates)();

	static CONST PassTemplate* METHOD(FindTemplate)(CONST String& name);

	static void METHOD(RegisterUserTemplate)(CONST PassTemplate& tmpl);

	static void METHOD(LoadUserTemplates)(CONST String& dir_path);

private:
public:
	static Vector<PassTemplate> s_builtin;
	static Vector<PassTemplate> s_user;
MYRENDERER_END_CLASS

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
