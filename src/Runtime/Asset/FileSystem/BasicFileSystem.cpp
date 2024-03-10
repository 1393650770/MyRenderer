#include "BasicFileSystem.h"
#include "Core/ConstDefine.h"
MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Asset)

String BasicFile::GetOpenModeStr()
{
	String OpenModeStr;
	switch (open_attribs.access_mode)
	{
		// clang-format off
	case EFileAccessMode::Read:            OpenModeStr += 'r'; break;
	case EFileAccessMode::Overwrite:       OpenModeStr += 'w'; break;
	case EFileAccessMode::Append:          OpenModeStr += 'a'; break;
	case EFileAccessMode::ReadUpdate:      OpenModeStr += "r+"; break;
	case EFileAccessMode::OverwriteUpdate: OpenModeStr += "w+"; break;
	case EFileAccessMode::AppendUpdate:    OpenModeStr += "a+"; break;
		// clang-format on
	default: break;
	}

	// Always open file in binary mode. Text mode is platform-specific
	OpenModeStr += 'b';

	return OpenModeStr;
}


BasicFile* BasicFileSystem::OpenFile(FileOpenAttribs& open_attribs)
{
	return nullptr;
}

void BasicFileSystem::ReleaseFile(BasicFile* in_file)
{
	if (in_file)
		delete in_file;
}

Bool BasicFileSystem::FileExists(CONST Char* strFilePath)
{
	return false;
}

void BasicFileSystem::CorrectSlashes(String& Path, Char Slash /*= 0*/)
{
	if (Slash != 0)
		CHECK_WITH_LOG(!IsSlash(Slash), "File System: Incorrect slash symbol")
	else
		Slash = SlashSymbol;
	Char RevSlashSym = (Slash == '\\') ? '/' : '\\';
	std::replace(Path.begin(), Path.end(), RevSlashSym, Slash);
}

void BasicFileSystem::GetPathComponents(CONST String& Path, String* Directory, String* FileName)
{
	auto LastSlashPos = Path.find_last_of("/\\");
	if (Directory)
	{
		if (LastSlashPos != String::npos)
			*Directory = Path.substr(0, LastSlashPos);
		else
			*Directory = "";
	}

	if (FileName)
	{
		if (LastSlashPos != String::npos)
			*FileName = Path.substr(LastSlashPos + 1);
		else
			*FileName = Path;
	}
}

Bool BasicFileSystem::IsPathAbsolute(CONST Char* strPath)
{
	if (strPath == nullptr || strPath[0] == 0)
		return false;

#if PLATFORM_WIN32 || PLATFORM_UNIVERSAL_WINDOWS
	return ((strPath[1] == ':' && (strPath[2] == '\\' || strPath[2] == '/')) || // c:\Path or c:/Path
		(strPath[0] == '\\' && strPath[1] == '\\'));                        // \\?\Path
#elif PLATFORM_LINUX || PLATFORM_MACOS || PLATFORM_IOS || PLATFORM_TVOS || PLATFORM_ANDROID || PLATFORM_EMSCRIPTEN
	return strPath[0] == '/';
#else
#    error Unknown platform.
#endif
}
template <typename StringType>
Vector<StringType> SplitPath(CONST Char* Path, Bool Simplify)
{
	std::vector<StringType> Components;

	// BasicFileSystem::IsSlash() does not get inlined by at least MSVC
	auto IsSlash = [](Char c) {
		return c == '/' || c == '\\';
	};

	// Estimate the number of components and reserve space in the vector
	{
		size_t CompnentCount = 1;
		for (const auto* c = Path; *c != '\0'; ++c)
		{
			if (IsSlash(c[0]) && !IsSlash(c[1]))
				++CompnentCount;
		}
		Components.reserve(CompnentCount);
	}

	const auto* c = Path;
	while (*c != '\0')
	{
		while (IsSlash(*c))
			++c;

		if (*c == '\0')
		{
			// a/
			break;
		}

		const auto* const CmpStart = c;
		while (*c != '\0' && !IsSlash(*c))
			++c;

		if (Simplify)
		{
			if ((c - CmpStart) == 1 && CmpStart[0] == '.') // "."
			{
				// Skip /.
				continue;
			}
			else if ((c - CmpStart) == 2 && CmpStart[0] == '.' && CmpStart[1] == '.') // ".."
			{
				// Pop previous subdirectory if "/.." is found, but only if there is
				// no ".." already (e.g "../..")
				if (!Components.empty() && Components.back() != "..")
				{
					Components.pop_back();
					continue;
				}
			}
		}

		Components.emplace_back(CmpStart, c);
	}

	return Components;
}

Vector<String> BasicFileSystem::SplitPath(CONST Char* Path, Bool Simplify)
{
	return MXRender::Asset::SplitPath<String>(Path, Simplify);
}


String BasicFileSystem::BuildPathFromComponents(CONST std::vector<String>& Components, Char Slash /*= 0*/)
{
	if (Slash != 0)
		CHECK_WITH_LOG(!IsSlash(Slash), "File System: Incorrect slash symbol")
	else
		Slash = SlashSymbol;

	String Res;
	for (size_t i = 0; i < Components.size(); ++i)
	{
		Res += Components[i];
		if (i < Components.size() - 1)
			Res += Slash;
	}
	return Res;
}

String BasicFileSystem::SimplifyPath(CONST Char* Path, Char Slash /*= 0*/)
{
	if (Path == nullptr)
		return "";

	if (Slash != 0)
		CHECK_WITH_LOG(!IsSlash(Slash), "File System : Incorrect slash symbol")
	else
		Slash = SlashSymbol;

	struct MiniStringView
	{
		MiniStringView(const char* _Start,
			const char* _End) :
			Start{ _Start },
			End{ _End }
		{}

		bool operator==(const char* Str) const noexcept
		{
			const auto Len = End - Start;
			return strncmp(Str, Start, Len) == 0 && Str[Len] == '\0';
		}

		bool operator!=(const char* str) const noexcept
		{
			return !(*this == str);
		}

		const char* const Start;
		const char* const End;
	};

	const auto PathComponents = MXRender::Asset::SplitPath<MiniStringView>(Path, true);
	const auto NumComponents = PathComponents.size();
	const auto UseLeadingSlash = Slash == '/' && IsSlash(Path[0]);

	size_t Len = UseLeadingSlash ? 1 : 0;
	for (const auto& Cmp : PathComponents)
		Len += Cmp.End - Cmp.Start;
	if (NumComponents > 0)
		Len += NumComponents - 1;

	std::string SimplifiedPath;
	SimplifiedPath.reserve(Len);
	if (UseLeadingSlash)
		SimplifiedPath.push_back(Slash);

	for (size_t i = 0; i < NumComponents; ++i)
	{
		if (i > 0)
			SimplifiedPath.push_back(Slash);
		const auto& Cmp = PathComponents[i];
		SimplifiedPath.append(Cmp.Start, Cmp.End);
	}
	//VERIFY_EXPR(SimplifiedPath.length() == Len);

	return SimplifiedPath;
}

String BasicFileSystem::GetRelativePath(CONST Char* PathFrom, Bool IsFromDirectory, CONST Char* PathTo, Bool IsToDirectory)
{
	CHECK_WITH_LOG(PathFrom == nullptr, "File System : Source path must not be null");
	CHECK_WITH_LOG(PathTo == nullptr, "File System : Destination path must not be null");

	const auto FromPathComps = SplitPath(PathFrom, true);
	const auto ToPathComps = SplitPath(PathTo, true);

	auto from_it = FromPathComps.begin();
	auto to_it = ToPathComps.begin();
	while (from_it != FromPathComps.end() && to_it != ToPathComps.end() && *from_it == *to_it)
	{
		++from_it;
		++to_it;
	}
	if (from_it == FromPathComps.begin())
		return PathFrom; // No common prefix

	String RelPath;
	for (; from_it != FromPathComps.end(); ++from_it)
	{
		if (!IsFromDirectory && from_it + 1 == FromPathComps.end())
		{
			//                    from_it
			//                       V
			// from:    "common/from/file"
			// to:      "common/to"
			// RelPath: ".."
			break;
		}

		if (!RelPath.empty())
			RelPath.push_back(SlashSymbol);
		RelPath.append("..");
	}

	for (; to_it != ToPathComps.end(); ++to_it)
	{
		// IsToDirectory is in fact irrelevant
		if (!RelPath.empty())
			RelPath.push_back(SlashSymbol);
		RelPath.append(*to_it);
	}

	return RelPath;
}

String BasicFileSystem::FileDialog(CONST FileDialogAttribs& DialogAttribs)
{
	return "";
}

String BasicFileSystem::OpenFolderDialog(CONST char* Title)
{
	return "";
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE