#pragma once
#ifndef _PLATFORM_FILEDIALOG_
#define _PLATFORM_FILEDIALOG_
#include "Core/ConstDefine.h"
#include "Platform/Platform.h"

#if PLATFORM_WIN32
#include "Platform/Win/WinFileDialog.h"
#else
// Stubs for non-Windows platforms
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)
inline String OpenFileDialog(CONST String& = "") { return ""; }
inline String SaveFileDialog(CONST String& = "") { return ""; }
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif

#endif
