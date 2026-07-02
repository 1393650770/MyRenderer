#pragma once
#ifndef _RENDERGRAPHNODECOLORS_
#define _RENDERGRAPHNODECOLORS_

#include "Core/ConstDefine.h"
#include <imgui.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// Pass type classification for visual differentiation
enum class PassNodeType : UInt8
{
	Graphics,
	Compute,
	Copy,
	UI,
	Custom
};

// Resource type for visual differentiation
enum class ResourceNodeType : UInt8
{
	Texture,
	Buffer,
	ExternalTexture,
	DepthStencil
};

// Pin access semantics — what this pin represents in the RDG dataflow
enum class PinAccess : UInt8
{
	Read,    // Pass reads this resource (blue)
	Write,   // Pass writes to this resource (yellow)
	Create   // Pass creates this resource (green)
};

// Centralized color palette for RenderGraph node editor
namespace RenderGraphColors
{
	// === Pass node header colors ===
	inline ImColor GetPassHeaderColor(PassNodeType type)
	{
		switch (type)
		{
		case PassNodeType::Graphics: return ImColor(50, 100, 200, 255);   // Deep blue
		case PassNodeType::Compute:  return ImColor(50, 170, 70, 255);    // Deep green
		case PassNodeType::Copy:     return ImColor(190, 160, 40, 255);   // Deep yellow
		case PassNodeType::UI:       return ImColor(130, 70, 190, 255);   // Deep purple
		case PassNodeType::Custom:   return ImColor(100, 100, 110, 255);  // Deep gray
		default:                     return ImColor(100, 100, 110, 255);
		}
	}

	inline const Char* GetPassTypeName(PassNodeType type)
	{
		switch (type)
		{
		case PassNodeType::Graphics: return "Graphics";
		case PassNodeType::Compute:  return "Compute";
		case PassNodeType::Copy:     return "Copy";
		case PassNodeType::UI:       return "UI";
		case PassNodeType::Custom:   return "Custom";
		default:                     return "Unknown";
		}
	}

	// === Resource node colors ===
	inline ImColor GetResourceBorderColor(ResourceNodeType type, bool is_transient)
	{
		if (!is_transient)
			return ImColor(255, 255, 255, 200); // White border for external/imported
		switch (type)
		{
		case ResourceNodeType::Texture:        return ImColor(60, 200, 180, 255); // Teal
		case ResourceNodeType::Buffer:         return ImColor(200, 150, 50, 255); // Orange
		case ResourceNodeType::ExternalTexture:return ImColor(255, 255, 255, 200); // White
		case ResourceNodeType::DepthStencil:   return ImColor(200, 80, 80, 255);  // Red-ish
		default:                               return ImColor(150, 150, 150, 255);
		}
	}

	inline ImColor GetResourceHeaderColor(ResourceNodeType type)
	{
		switch (type)
		{
		case ResourceNodeType::Texture:        return ImColor(40, 140, 120, 255); // Darker teal
		case ResourceNodeType::Buffer:         return ImColor(160, 110, 30, 255); // Darker orange
		case ResourceNodeType::ExternalTexture:return ImColor(80, 80, 80, 255);   // Dark gray
		case ResourceNodeType::DepthStencil:   return ImColor(140, 50, 50, 255);  // Darker red
		default:                               return ImColor(100, 100, 100, 255);
		}
	}

	inline const Char* GetResourceTypeName(ResourceNodeType type)
	{
		switch (type)
		{
		case ResourceNodeType::Texture:        return "Texture";
		case ResourceNodeType::Buffer:         return "Buffer";
		case ResourceNodeType::ExternalTexture:return "External";
		case ResourceNodeType::DepthStencil:   return "DepthStencil";
		default:                               return "Unknown";
		}
	}

	// === Pin colors by access type ===
	inline ImColor GetPinColor(PinAccess access)
	{
		switch (access)
		{
		case PinAccess::Read:   return ImColor(80, 160, 255, 255); // Light blue
		case PinAccess::Write:  return ImColor(220, 200, 60, 255); // Yellow
		case PinAccess::Create: return ImColor(80, 220, 80, 255);  // Green
		default:                return ImColor(200, 200, 200, 255);
		}
	}

	inline const Char* GetPinAccessName(PinAccess access)
	{
		switch (access)
		{
		case PinAccess::Read:   return "Read";
		case PinAccess::Write:  return "Write";
		case PinAccess::Create: return "Create";
		default:                return "?";
		}
	}

	// === Link colors ===
	inline ImColor GetLinkColor(PinAccess access)
	{
		switch (access)
		{
		case PinAccess::Read:   return ImColor(80, 160, 255, 180);  // Blue-ish
		case PinAccess::Write:  return ImColor(220, 200, 60, 180);  // Yellow-ish
		case PinAccess::Create: return ImColor(80, 220, 80, 180);   // Green-ish
		default:                return ImColor(200, 200, 200, 180);
		}
	}

	// === Status colors ===
	inline ImColor GetCulledColor()     { return ImColor(180, 50, 50, 200); }  // Red overlay
	inline ImColor GetActiveColor()     { return ImColor(50, 255, 50, 200); }  // Green pulse
	inline ImColor GetCompletedColor()  { return ImColor(50, 200, 50, 180); }  // Green check
	inline ImColor GetPendingColor()    { return ImColor(120, 120, 120, 150); } // Grayed out

	// === Separator line in nodes ===
	inline ImColor GetSeparatorColor()  { return ImColor(255, 255, 255, 200); }
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif // !_RENDERGRAPHNODECOLORS_
