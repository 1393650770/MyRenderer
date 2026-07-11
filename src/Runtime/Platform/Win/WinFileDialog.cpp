// --   --
#include "Platform/FileDialog.h"
#include <windows.h>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)

// Internal helper: build null-separated filter string for Win32 OPENFILENAME
static String BuildFilter(const String& desc, const String& ext)
{
	String f = desc;
	f.push_back('\0');
	f.append(ext);
	f.push_back('\0');
	return f;
}

String OpenFileDialog(const String& ext_filter)
{
	String filter = BuildFilter("RenderGraph JSON",
		ext_filter.empty() ? "*.rgraph.json" : ext_filter);
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

String SaveFileDialog(const String& ext_filter)
{
	String filter = BuildFilter("RenderGraph JSON",
		ext_filter.empty() ? "*.rgraph.json" : ext_filter);
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
// --   --
