#pragma once
#ifndef _RENDERGRAPH_PASSTEMPLATE_
#define _RENDERGRAPH_PASSTEMPLATE_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "UI/RenderGraphEditor/Core/RenderGraphNodeColors.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// --   Template pin descriptor
MYRENDERER_BEGIN_STRUCT(TemplatePinDef)
public:
	String pin_name;
	PinAccess access = PinAccess::Read;
	Render::RDGResourceKind resource_kind = Render::RDGResourceKind::Texture;
MYRENDERER_END_STRUCT

// --   Reusable pass template definition
MYRENDERER_BEGIN_STRUCT(PassTemplate)
public:
	String name;
	String description;
	String category;
	Render::RDGPassKind pass_kind = Render::RDGPassKind::Graphics;
	Vector<TemplatePinDef> input_pins;
	Vector<TemplatePinDef> output_pins;
	// --   Auto-created resource nodes when applying this template
	Vector<Render::RDGResourceDef> auto_create_resources;
MYRENDERER_END_STRUCT

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
