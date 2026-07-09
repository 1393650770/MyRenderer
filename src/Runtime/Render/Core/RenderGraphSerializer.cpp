#include "Render/Core/RenderGraphSerializer.h"
#include "Render/Core/RenderGraphDefinition.h"
#include "Render/Core/RenderGraphResourceImplementation.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

String RenderGraphSerializer::s_last_error;

Bool RenderGraphSerializer::SaveGraph(CONST RenderGraphDefinition& def, CONST String& filepath)
{
	s_last_error.clear();
	try
	{
		json j;

		j["graph_name"] = def.graph_name;
		j["version"] = def.version;
		j["editor_zoom"] = def.editor_zoom;
		j["editor_offset_x"] = def.editor_offset_x;
		j["editor_offset_y"] = def.editor_offset_y;

		// Serialize resources — use embedded RHI descriptor directly
		json resources_json = json::array();
		for (auto& rd : def.resources)
		{
			json rj;
			rj["name"] = rd.name;
			rj["kind"] = ResourceKindToString(rd.GetKind());
			rj["is_transient"] = rd.is_transient;

			rj["is_depth_stencil"] = rd.is_depth_stencil;
			std::visit([&](auto& d) {
				using T = std::decay_t<decltype(d)>;
				ResourceDescSerializer<T>::Serialize(rj, d);
			}, rd.desc);
			resources_json.push_back(rj);
		}
		j["resources"] = resources_json;

		// Serialize passes
		json passes_json = json::array();
		for (auto& pd : def.passes)
		{
			json pj;
			pj["name"] = pd.name;
			pj["pass_kind"] = PassKindToString(pd.pass_kind);
			pj["pass_flags"] = (UInt32)pd.pass_flags;
			pj["read_resources"] = pd.read_resources;
			pj["write_resources"] = pd.write_resources;
			pj["create_resources"] = pd.create_resources;
			passes_json.push_back(pj);
		}
		j["passes"] = passes_json;

		// Serialize node layouts
		json layouts_json = json::array();
		for (auto& nl : def.node_layouts)
		{
			json lj;
			lj["node_name"] = nl.node_name;
			lj["pos_x"] = nl.pos_x;
			lj["pos_y"] = nl.pos_y;
			layouts_json.push_back(lj);
		}
		j["node_layouts"] = layouts_json;
	// Serialize edges
		json edges_json = json::array();
		for (auto& ed : def.edges)
		{
			json ej;
			ej["source_node"] = ed.source_node_name;
			ej["source_pin"] = ed.source_pin_name;
			ej["target_node"] = ed.target_node_name;
			ej["target_pin"] = ed.target_pin_name;
			ej["edge_type"] = ed.edge_type;
			edges_json.push_back(ej);
		}
		j["edges"] = edges_json;

		// Write to file
		std::ofstream file(filepath);
		if (!file.is_open())
		{
			s_last_error = "Failed to open file for writing: " + filepath;
			return false;
		}
		file << j.dump(2); // Pretty-print with 2-space indent
		file.close();

		return true;
	}
	catch (const std::exception& e)
	{
		s_last_error = String("JSON serialization error: ") + e.what();
		return false;
	}
}

Bool RenderGraphSerializer::LoadGraph(RenderGraphDefinition& out_def, CONST String& filepath)
{
	s_last_error.clear();
	try
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			s_last_error = "Failed to open file for reading: " + filepath;
			return false;
		}

		json j;
		file >> j;
		file.close();

		out_def = RenderGraphDefinition();

		if (j.contains("graph_name"))
			out_def.graph_name = j["graph_name"].get<String>();
		if (j.contains("version"))
			out_def.version = j["version"].get<UInt32>();
		if (j.contains("editor_zoom"))
			out_def.editor_zoom = j["editor_zoom"].get<float>();
		if (j.contains("editor_offset_x"))
			out_def.editor_offset_x = j["editor_offset_x"].get<float>();
		if (j.contains("editor_offset_y"))
			out_def.editor_offset_y = j["editor_offset_y"].get<float>();

		// Deserialize resources -- populate embedded RHI descriptor from JSON
		if (j.contains("resources") && j["resources"].is_array())
		{
			for (auto& rj : j["resources"])
			{
				RDGResourceDef rd;
				rd.name = rj.value("name", "Unnamed");
				rd.is_transient = rj.value("is_transient", true);
				rd.is_depth_stencil = rj.value("is_depth_stencil", false);
				auto kind = StringToResourceKind(rj.value("kind", "Texture"));

				if (kind == RDGResourceKind::Buffer)
					rd.desc = ResourceDescSerializer<RHI::BufferDesc>::Deserialize(rj);
				else if (rj.contains("spirv_data"))
					rd.desc = ResourceDescSerializer<RHI::ShaderDesc>::Deserialize(rj);
				else
					rd.desc = ResourceDescSerializer<RHI::TextureDesc>::Deserialize(rj);
				out_def.resources.push_back(rd);
			}
		}

		// Deserialize passes
		if (j.contains("passes") && j["passes"].is_array())
		{
			for (auto& pj : j["passes"])
			{
				RDGPassDef pd;
				pd.name = pj.value("name", "Unnamed");
				pd.pass_kind = StringToPassKind(pj.value("pass_kind", "Graphics"));
				pd.pass_flags = (RDGPassFlags)pj.value("pass_flags", (UInt32)RDGPassFlags::Raster);

				if (pj.contains("read_resources") && pj["read_resources"].is_array())
					for (auto& r : pj["read_resources"])
						pd.read_resources.push_back(r.get<String>());

				if (pj.contains("write_resources") && pj["write_resources"].is_array())
					for (auto& r : pj["write_resources"])
						pd.write_resources.push_back(r.get<String>());

				if (pj.contains("create_resources") && pj["create_resources"].is_array())
					for (auto& r : pj["create_resources"])
						pd.create_resources.push_back(r.get<String>());

				out_def.passes.push_back(pd);
			}
		}

		// Deserialize node layouts
		if (j.contains("node_layouts") && j["node_layouts"].is_array())
		{
			for (auto& lj : j["node_layouts"])
			{
				RDGNodeLayout nl;
				nl.node_name = lj.value("node_name", "");
				nl.pos_x = lj.value("pos_x", 0.0f);
				nl.pos_y = lj.value("pos_y", 0.0f);
				out_def.node_layouts.push_back(nl);
			}
		}
	// Deserialize edges
		if (j.contains("edges") && j["edges"].is_array())
		{
			for (auto& ej : j["edges"])
			{
				RDGEdgeDef ed;
				ed.source_node_name = ej.value("source_node", "");
				ed.source_pin_name = ej.value("source_pin", "");
				ed.target_node_name = ej.value("target_node", "");
				ed.target_pin_name = ej.value("target_pin", "");
				ed.edge_type = ej.value("edge_type", 0);
				out_def.edges.push_back(ed);
			}
		}

		return true;
	}
	catch (const std::exception& e)
	{
		s_last_error = String("JSON deserialization error: ") + e.what();
		return false;
	}
}

// ---- Enum conversion helpers ----

CONST Char* RenderGraphSerializer::TextureFormatToString(Int format)
{
	static CONST Char* names[] = {
		"None","BC1","BC1A","BC2","BC3","BC4","BC5","BC6H","BC7",
		"ETC1","ETC2","ETC2A","ETC2A1","PTC12","PTC14","PTC12A","PTC14A","PTC22","PTC24",
		"ATC","ATCE","ATCI","ASTC4x4","ASTC5x5","ASTC6x6","ASTC8x5","ASTC8x6","ASTC10x5",
		"Unknown",
		"R1","A8","R8","R8I","R8U","R8S","R16","R16I","R16U","R16F","R16S",
		"R32I","R32U","R32F","RG8","RG8I","RG8U","RG8S","RG16","RG16I","RG16U","RG16F","RG16S",
		"RG32I","RG32U","RG32F","RGB8","RGB8I","RGB8U","RGB8S","RGB9E5F",
		"RGB16I","RGB16U","RGB16F","RGB32I","RGB32U","RGB32F",
		"BGRA8","RGBA8","RGBA8I","RGBA8U","RGBA8S","RGBA16","RGBA16I","RGBA16U","RGBA16F","RGBA16S",
		"RGBA32I","RGBA32U","RGBA32F","R5G6B5","RGBA4","RGB5A1","RGB10A2","RG11B10F",
		"UnknownDepth",
		"D16","D24","D24S8","D32","D32FS8","D16F","D24F","D32F","D0S8"
	};
	if (format < 0 || format >= (Int)(sizeof(names) / sizeof(names[0])))
		return "RGBA8";
	return names[format];
}

Int RenderGraphSerializer::StringToTextureFormat(CONST String& str)
{
	static CONST Char* names[] = {
		"None","BC1","BC1A","BC2","BC3","BC4","BC5","BC6H","BC7",
		"ETC1","ETC2","ETC2A","ETC2A1","PTC12","PTC14","PTC12A","PTC14A","PTC22","PTC24",
		"ATC","ATCE","ATCI","ASTC4x4","ASTC5x5","ASTC6x6","ASTC8x5","ASTC8x6","ASTC10x5",
		"Unknown",
		"R1","A8","R8","R8I","R8U","R8S","R16","R16I","R16U","R16F","R16S",
		"R32I","R32U","R32F","RG8","RG8I","RG8U","RG8S","RG16","RG16I","RG16U","RG16F","RG16S",
		"RG32I","RG32U","RG32F","RGB8","RGB8I","RGB8U","RGB8S","RGB9E5F",
		"RGB16I","RGB16U","RGB16F","RGB32I","RGB32U","RGB32F",
		"BGRA8","RGBA8","RGBA8I","RGBA8U","RGBA8S","RGBA16","RGBA16I","RGBA16U","RGBA16F","RGBA16S",
		"RGBA32I","RGBA32U","RGBA32F","R5G6B5","RGBA4","RGB5A1","RGB10A2","RG11B10F",
		"UnknownDepth",
		"D16","D24","D24S8","D32","D32FS8","D16F","D24F","D32F","D0S8"
	};
	Int count = sizeof(names) / sizeof(names[0]);
	for (Int i = 0; i < count; ++i)
	{
		if (str == names[i]) return i;
	}
	return 0; // None
}

CONST Char* RenderGraphSerializer::ResourceKindToString(RDGResourceKind kind)
{
	switch (kind)
	{
	case RDGResourceKind::Texture:        return "Texture";
	case RDGResourceKind::Buffer:         return "Buffer";
	case RDGResourceKind::ExternalTexture:return "ExternalTexture";
	case RDGResourceKind::DepthStencil:   return "DepthStencil";
	default: return "Texture";
	}
}

RDGResourceKind RenderGraphSerializer::StringToResourceKind(CONST String& str)
{
	if (str == "Buffer")          return RDGResourceKind::Buffer;
	if (str == "ExternalTexture") return RDGResourceKind::ExternalTexture;
	if (str == "DepthStencil")    return RDGResourceKind::DepthStencil;
	return RDGResourceKind::Texture;
}

CONST Char* RenderGraphSerializer::PassKindToString(RDGPassKind kind)
{
	switch (kind)
	{
	case RDGPassKind::Graphics: return "Graphics";
	case RDGPassKind::Compute:  return "Compute";
	case RDGPassKind::Copy:     return "Copy";
	case RDGPassKind::Custom:   return "Custom";
	default: return "Graphics";
	}
}

RDGPassKind RenderGraphSerializer::StringToPassKind(CONST String& str)
{
	if (str == "Compute")  return RDGPassKind::Compute;
	if (str == "Copy")     return RDGPassKind::Copy;
	if (str == "Custom")   return RDGPassKind::Custom;
	return RDGPassKind::Graphics;
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
