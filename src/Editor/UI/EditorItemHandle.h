#pragma once
#ifndef _EDITORITEMHANDLE_
#define _EDITORITEMHANDLE_

#include "Core/ResourceHandle.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

struct TagEditorNode {};
struct TagEditorPin {};
struct TagEditorLink {};
using NodeHandle = ResourceHandle<TagEditorNode>;
using PinHandle  = ResourceHandle<TagEditorPin>;
using LinkHandle = ResourceHandle<TagEditorLink>;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
