#pragma once
#ifndef _RENDERGRAPHDEFINITION_
#define _RENDERGRAPHDEFINITION_

#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Pure-data description of a render graph, no lambdas, no runtime objects.
// This is the intermediate representation between the editor and the runtime.
// Used for serialization (JSON) and runtime graph construction.

enum class RDGResourceLifetime : UInt8
{
	Transient,    // Created and consumed within graph; can alias memory
	External,     // Imported from outside; lifetime beyond graph
	History,      // External resource from previous frame(s)
};

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
	RDGResourceLifetime lifetime = RDGResourceLifetime::Transient;

	// Texture properties
	Int texture_format = 0;        // TextureFomat enum value
	UInt32 width = 1920;
	UInt32 height = 1080;
	UInt8 mip_level = 1;
	UInt8 layer_count = 1;
	UInt8 samples = 1;
	UInt8 texture_type = 0;       // ENUM_TEXTURE_TYPE (2D/3D/Cube/Array)

	// Buffer properties
	UInt64 buffer_size = 256;
	UInt32 buffer_stride = 16;
	UInt8 buffer_type = 0;        // ENUM_BUFFER_TYPE

	// Common
	UInt32 usage = 0;
	UInt16 depth = 1;

	// Lifecycle
	Bool is_transient = true;      // Created/destroyed within graph
	Bool is_depth_stencil = false;
};

enum class RDGPassFlags : UInt32
{
	None           = 0,
	Raster         = 1 << 0,
	Compute        = 1 << 1,
	AsyncCompute   = 1 << 2,
	Copy           = 1 << 3,
	NeverCull      = 1 << 4,
	SkipRenderPass = 1 << 5,
	NeverMerge     = 1 << 6,
	NeverParallel  = 1 << 7,
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
	RDGPassFlags pass_flags = RDGPassFlags::Raster;
	String shader_path;

	// Resource references (by name)
	Vector<String> read_resources;
	Vector<String> write_resources;
	Vector<String> create_resources;

	Vector<String> explicit_dependencies;
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

struct RDGSubGraphDef
{
	String name;
	Vector<String> contained_pass_names;
	Vector<String> exposed_inputs;
	Vector<String> exposed_outputs;
	float pos_x = 0, pos_y = 0, width = 300, height = 200;
	UInt32 color = 0x80808080;
};

struct RDGAnnotationDef
{
	String text;
	float pos_x = 0, pos_y = 0, width = 200, height = 80;
	UInt32 color = 0xFFFFFF80;
};

struct RDGGraphMetadata
{
	String author;
	String description;
	String created_at;
	String updated_at;
};

// Complete graph definition (pure data, serializable)
struct RenderGraphDefinition
{
	String graph_name = "RenderGraph";
	UInt32 version = 2;

	Vector<RDGPassDef> passes;
	Vector<RDGResourceDef> resources;
	Vector<RDGEdgeDef> edges;
	Vector<RDGNodeLayout> node_layouts;

	Vector<RDGSubGraphDef> sub_graphs;
	Vector<RDGAnnotationDef> annotations;
	RDGGraphMetadata metadata;

	// Editor state
	float editor_zoom = 1.0f;
	float editor_offset_x = 0.0f;
	float editor_offset_y = 0.0f;
};

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif // !_RENDERGRAPHDEFINITION_
