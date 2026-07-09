#pragma once
#ifndef _WIN_FILEDIALOG_
#define _WIN_FILEDIALOG_
#include "Core/ConstDefine.h"
#include "Platform/FileDialog.h"
#include <windows.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)

// Helper: build null-separated filter string for Win32 dialog
inline String BuildFilter(CONST String& desc, CONST String& ext)
{
	String f = desc;
	f.push_back('\0');
	f.append(ext);
	f.push_back('\0');
	return f;
}

inline String OpenFileDialog(CONST String& ext_filter = "*.rgraph.json")
{
	String filter = BuildFilter("RenderGraph JSON", ext_filter);
	CHAR buf[512] = {};
	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = buf;
	ofn.nMaxFile = sizeof(buf);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileNameA(&ofn)) return String(buf);
	return "";
}

inline String SaveFileDialog(CONST String& ext_filter = "*.rgraph.json")
{
	String filter = BuildFilter("RenderGraph JSON", ext_filter);
	CHAR buf[512] = {};
	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = filter.c_str();
	ofn.lpstrFile = buf;
	ofn.nMaxFile = sizeof(buf);
	ofn.lpstrDefExt = "rgraph.json";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	if (GetSaveFileNameA(&ofn)) return String(buf);
	return "";
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
