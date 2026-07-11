#pragma once
#ifndef _PLATFORM_FILEDIALOG_
#define _PLATFORM_FILEDIALOG_
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)

// ----   Cross-platform file dialog API ----
// Declarations only — platform-specific implementations live in:
//   Windows:  Platform/Win/WinFileDialog.cpp
//   Other:    Platform/FileDialogStub.cpp

String OpenFileDialog(const String& ext_filter = "");
String SaveFileDialog(const String& ext_filter = "");

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
