#pragma once
#ifndef _FILESYSTEM_
#define _FILESYSTEM_
#include "Core/ConstDefine.h"

#if PLATFORM_WIN32

#include "Asset/FileSystem/Win/WinFileSystem.h"

#elif PLATFORM_UNIVERSAL_WINDOWS


#elif PLATFORM_ANDROID


#elif PLATFORM_LINUX


#elif PLATFORM_MACOS || PLATFORM_IOS || PLATFORM_TVOS


#elif PLATFORM_EMSCRIPTEN


#else

#endif

MYRENDERER_BEGIN_NAMESPACE(MXRender)

#if PLATFORM_WIN32

using FileSystem = WindowsFileSystem;
using CFile = WindowsFile;

#elif PLATFORM_UNIVERSAL_WINDOWS


#elif PLATFORM_ANDROID


#elif PLATFORM_LINUX



#elif PLATFORM_MACOS || PLATFORM_IOS || PLATFORM_TVOS


#elif PLATFORM_EMSCRIPTEN


#else

#endif

MYRENDERER_END_NAMESPACE // namespace Diligent

#endif
