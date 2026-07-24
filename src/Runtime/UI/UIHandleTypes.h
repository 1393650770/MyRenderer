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

// ─── Generic UI handles (backend-agnostic) ─────────────────────────────────
// Application code uses these.  The RmlUI namespace aliases below provide
// source-compatible access for existing code.

struct TagUIModel {};
struct TagUIDoc   {};

using UIModelHandle = ResourceHandle<TagUIModel>;
using UIDocHandle   = ResourceHandle<TagUIDoc>;

// ─── Backward-compatible aliases (RmlUI backend) ───────────────────────────
// These share the same underlying Tag types so handles are interchangeable.
// Existing code using RmlModelHandle / RmlDocHandle continues to compile.

MYRENDERER_BEGIN_NAMESPACE(RmlUI)

using RmlModelHandle = UIModelHandle;      // alias, same ResourceHandle<TagUIModel>
using RmlDocHandle   = UIDocHandle;        // alias, same ResourceHandle<TagUIDoc>

MYRENDERER_END_NAMESPACE // RmlUI

MYRENDERER_END_NAMESPACE // UI
MYRENDERER_END_NAMESPACE // MXRender
#endif // !_UIHANDLETYPES_
