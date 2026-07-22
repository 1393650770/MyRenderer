#pragma once
#ifndef _UIHANDLETYPES_
#define _UIHANDLETYPES_

#include "Core/ResourceHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

// Tag types for UI resource handles (see convention 12.8)
struct TagUIGeometry {};
struct TagUITexture {};
struct TagUIFilter {};

using UIGeometryHandle = ResourceHandle<TagUIGeometry>;
using UITextureHandle  = ResourceHandle<TagUITexture>;
using UIFilterHandle   = ResourceHandle<TagUIFilter>;

MYRENDERER_BEGIN_NAMESPACE(RmlUI)

struct TagRmlModel {};
struct TagRmlDoc   {};

using RmlModelHandle = ResourceHandle<TagRmlModel>;
using RmlDocHandle   = ResourceHandle<TagRmlDoc>;

MYRENDERER_END_NAMESPACE // RmlUI

MYRENDERER_END_NAMESPACE // UI
MYRENDERER_END_NAMESPACE // MXRender
#endif
