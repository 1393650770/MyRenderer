#pragma once
#ifndef _BASICFILESYSTEM_
#define _BASICFILESYSTEM_
#include "Core/ConstDefine.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)


enum class EFileAccessMode
{
	Read,
	Overwrite,
	Append,
	ReadUpdate,
	OverwriteUpdate,
	AppendUpdate
};

enum class FilePosOrigin
{
	Start,
	Curr,
	End
};

MYRENDERER_BEGIN_STRUCT(FileOpenAttribs)
public:
	CONST Char* str_filepath;
	EFileAccessMode access_mode;
	FileOpenAttribs(CONST Char* in_path = nullptr,
		EFileAccessMode in_access = EFileAccessMode::Read) :
		str_filepath{ in_path },
		access_mode{ in_access }
	{}
MYRENDERER_END_STRUCT

MYRENDERER_BEGIN_CLASS(BasicFile)
public:
	BasicFile(CONST FileOpenAttribs& open_attribs);
	VIRTUAL ~BasicFile() {};

	CONST String& GetPath() { return path; }

protected:
	String GetOpenModeStr();

	CONST String          path;
	CONST FileOpenAttribs open_attribs;
MYRENDERER_END_CLASS


enum FILE_DIALOG_FLAGS : UInt32
{
	FILE_DIALOG_FLAG_NONE = 0x000,

	/// Prevents the system from adding a link to the selected file in the file system
	/// directory that contains the user's most recently used documents.
	FILE_DIALOG_FLAG_DONT_ADD_TO_RECENT = 0x001,

	/// Only existing files can be opened
	FILE_DIALOG_FLAG_FILE_MUST_EXIST = 0x002,

	/// Restores the current directory to its original value if the user changed the
	/// directory while searching for files.
	FILE_DIALOG_FLAG_NO_CHANGE_DIR = 0x004,

	/// Causes the Save As dialog box to show a message box if the selected file already exists.
	FILE_DIALOG_FLAG_OVERWRITE_PROMPT = 0x008
};
ENUM_CLASS_FLAGS(FILE_DIALOG_FLAGS);

enum FILE_DIALOG_TYPE : UInt32
{
	FILE_DIALOG_TYPE_OPEN,
	FILE_DIALOG_TYPE_SAVE
};

MYRENDERER_BEGIN_STRUCT(FileDialogAttribs)
public:
	FILE_DIALOG_TYPE  Type = FILE_DIALOG_TYPE_OPEN;
	FILE_DIALOG_FLAGS Flags = FILE_DIALOG_FLAG_NONE;

	const char* Title = nullptr;
	const char* Filter = nullptr;

	FileDialogAttribs() noexcept {}

	explicit FileDialogAttribs(FILE_DIALOG_TYPE _Type) noexcept :
		Type{ _Type }
	{
		switch (Type)
		{
		case FILE_DIALOG_TYPE_OPEN:
			Flags = FILE_DIALOG_FLAG_DONT_ADD_TO_RECENT | FILE_DIALOG_FLAG_FILE_MUST_EXIST | FILE_DIALOG_FLAG_NO_CHANGE_DIR;
			break;

		case FILE_DIALOG_TYPE_SAVE:
			Flags = FILE_DIALOG_FLAG_DONT_ADD_TO_RECENT | FILE_DIALOG_FLAG_OVERWRITE_PROMPT | FILE_DIALOG_FLAG_NO_CHANGE_DIR;
			break;
		}
	}
MYRENDERER_END_CLASS


MYRENDERER_BEGIN_STRUCT(FindFileData)
public:
	virtual const Char* Name() const = 0;
	virtual bool        IsDirectory() const = 0;

	virtual ~FindFileData() {}
MYRENDERER_END_STRUCT


MYRENDERER_BEGIN_CLASS(BasicFileSystem)
public:
#if PLATFORM_WIN32 || PLATFORM_UNIVERSAL_WINDOWS
	static constexpr Char SlashSymbol = '\\';
#else
	static constexpr Char SlashSymbol = '/';
#endif

	static BasicFile* OpenFile(FileOpenAttribs& open_attribs);
	static void       ReleaseFile(BasicFile* in_file);

	static Bool FileExists(CONST Char* strFilePath);

	static void SetWorkingDirectory(CONST Char* strWorkingDir) { m_strWorkingDirectory = strWorkingDir; }

	static CONST String& GetWorkingDirectory() { return m_strWorkingDirectory; }

	static Bool IsSlash(Char c)
	{
		return c == '/' || c == '\\';
	}

	static void CorrectSlashes(String& Path, Char Slash = 0);

	static void GetPathComponents(CONST String& Path,
		String* Directory,
		String* FileName);

	static Bool IsPathAbsolute(CONST Char* Path);

	/// Splits path into individual components optionally simplifying it.
	///
	/// If Simplify is true:
	///     - Removes redundant slashes (a///b -> a/b)
	///     - Removes redundant . (a/./b -> a/b)
	///     - Collapses .. (a/b/../c -> a/c)
	static Vector<String> SplitPath(CONST Char* Path, Bool Simplify);

	/// Builds a path from the given components.
	static String BuildPathFromComponents(CONST std::vector<String>& Components, Char Slash = 0);

	/// Simplifies the path.

	/// The function performs the following path simplifications:
	/// - Normalizes slashes using the given slash symbol (a\b/c -> a/b/c)
	/// - Removes redundant slashes (a///b -> a/b)
	/// - Removes redundant . (a/./b -> a/b)
	/// - Collapses .. (a/b/../c -> a/c)
	/// - Removes trailing slashes (/a/b/c/ -> /a/b/c)
	/// - When 'Slash' is Windows slash ('\'), removes leading slashes (\a\b\c -> a\b\c)
	static String SimplifyPath(CONST Char* Path, Char Slash = 0);


	/// Splits a list of paths separated by a given separator and calls a user callback for every individual path.
	/// Empty paths are skipped.
	template <typename CallbackType>
	static void SplitPathList(CONST Char* PathList, CallbackType Callback, CONST char Separator = ';')
	{
		if (PathList == nullptr)
			return;

		CONST auto* Pos = PathList;
		while (*Pos != '\0')
		{
			while (*Pos != '\0' && *Pos == Separator)
				++Pos;
			if (*Pos == '\0')
				break;

			CONST auto* End = Pos + 1;
			while (*End != '\0' && *End != Separator)
				++End;

			if (!Callback(Pos, static_cast<size_t>(End - Pos)))
				break;

			Pos = End;
		}
	}

	/// Returns a relative path from one file or folder to another.

	/// \param [in]  PathFrom        - Path that defines the start of the relative path.
	///                                Must not be null.
	/// \param [in]  IsFromDirectory - Indicates if PathFrom is a directory.
	/// \param [in]  PathTo          - Path that defines the endpoint of the relative path.
	///                                Must not be null.
	/// \param [in]  IsToDirectory   - Indicates if PathTo is a directory.
	///
	/// \return                        Relative path from PathFrom to PathTo.
	///                                If no relative path exists, PathFrom is returned.
	static String GetRelativePath(CONST Char* PathFrom,
		Bool        IsFromDirectory,
		CONST Char* PathTo,
		Bool        IsToDirectory);

	static String FileDialog(CONST FileDialogAttribs& DialogAttribs);
	static String OpenFolderDialog(CONST char* Title);

protected:
	static String m_strWorkingDirectory;
MYRENDERER_END_CLASS


MYRENDERER_END_NAMESPACE

#endif
