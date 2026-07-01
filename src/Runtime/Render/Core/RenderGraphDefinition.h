#pragma once
#ifndef _RENDERGRAPHDEFINITION_
#define _RENDERGRAPHDEFINITION_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Pure-data description of a render graph — no lambdas, no runtime objects.
// This is the intermediate representation between the editor and the runtime.
// Used for serialization (JSON) and runtime graph construction.

// Resource type classification
enum class RDGResourceKind : UInt8
{
	Texture,
	Buffer,
	ExternalTexture,
	DepthStencil
};

// Resource descriptor
struct RDGResourceDef
{
	String name;
	RDGResourceKind kind = RDGResourceKind::Texture;

	// Texture properties
	Int texture_format = 0;        // TextureFomat enum value
	UInt32 width = 1920;
	UInt32 height = 1080;
	UInt8 mip_level = 1;
	UInt8 samples = 1;

	// Buffer properties
	UInt64 buffer_size = 256;
	UInt32 buffer_stride = 16;

	// Lifecycle
	Bool is_transient = true;      // Created/destroyed within graph
	Bool is_depth_stencil = false;
};

// Pass type classification
enum class RDGPassKind : UInt8
{
	Graphics,
	Compute,
	Copy,
	Custom
};

// Pass descriptor
struct RDGPassDef
{
	String name;
	RDGPassKind pass_kind = RDGPassKind::Graphics;
	Bool is_cullable = true;

	// Resource references (by name)
	Vector<String> read_resources;
	Vector<String> write_resources;
	Vector<String> create_resources;
};

// Edge (link) connecting two pins across nodes
struct RDGEdgeDef
{
	String source_node_name;
	String source_pin_name;
	String target_node_name;
	String target_pin_name;
	Int edge_type = 0;  // PinAccess: 0=Read, 1=Write, 2=Create
};

// Editor metadata for node positions
struct RDGNodeLayout
{
	String node_name;
	float pos_x = 0.0f;
	float pos_y = 0.0f;
};

// Complete graph definition (pure data, serializable)
struct RenderGraphDefinition
{
	String graph_name = "RenderGraph";
	UInt32 version = 1;

	Vector<RDGPassDef> passes;
	Vector<RDGResourceDef> resources;
	Vector<RDGEdgeDef> edges;
	Vector<RDGNodeLayout> node_layouts;

	// Editor state
	float editor_zoom = 1.0f;
	float editor_offset_x = 0.0f;
	float editor_offset_y = 0.0f;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHDEFINITION_
