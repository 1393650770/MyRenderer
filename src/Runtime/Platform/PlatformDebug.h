#pragma once

#ifndef _PLATFORM_DEBUG_
#define _PLATFORM_DEBUG_

#include "Core/ConstDefine.h"
#include <cstdarg>
#include <cstdio>

#if PLATFORM_ANDROID
#include <android/log.h>
#elif PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)

//  UE  FPlatformMisc::LowLevelOutputDebugString
// Platform HAL for debug output — all platform #if/#elif live ONLY in this file.
// Usage: Platform::DebugPrintf("Tag", "format %d", value);

inline void DebugPrintf(CONST Char* tag, CONST Char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_INFO, tag, fmt, args);
#elif PLATFORM_WINDOWS
	Char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, args);
	OutputDebugStringA(buf);
#else
	vprintf(fmt, args);
#endif
	va_end(args);
}

inline void DebugErrorf(CONST Char* tag, CONST Char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_ERROR, tag, fmt, args);
#elif PLATFORM_WINDOWS
	Char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, args);
	Char full_buf[1536];
	snprintf(full_buf, sizeof(full_buf), "[ERROR] %s\n", buf);
	OutputDebugStringA(full_buf);
#else
	fprintf(stderr, "[ERROR] ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
#endif
	va_end(args);
}

inline void DebugWarnf(CONST Char* tag, CONST Char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_WARN, tag, fmt, args);
#elif PLATFORM_WINDOWS
	Char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, args);
	Char full_buf[1536];
	snprintf(full_buf, sizeof(full_buf), "[WARN] %s\n", buf);
	OutputDebugStringA(full_buf);
#else
	fprintf(stderr, "[WARN] ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
#endif
	va_end(args);
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

#endif
