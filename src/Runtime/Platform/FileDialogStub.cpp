// --   Non-Windows stub — returns empty string for all file dialogs
#include "Platform/FileDialog.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Platform)

String OpenFileDialog(const String&)
{
	return "";
}

String SaveFileDialog(const String&)
{
	return "";
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
// --   --
