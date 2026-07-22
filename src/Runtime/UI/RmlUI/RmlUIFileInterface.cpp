#include "RmlUIFileInterface.h"

#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Core.h>
#include <fstream>
#include <iostream>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)
MYRENDERER_BEGIN_NAMESPACE(RmlUI)

// =========================================================================
// Internal Impl
// =========================================================================
class RmlUIFileInterface::Impl : public Rml::FileInterface
{
public:
	Rml::FileHandle Open(const Rml::String& path) override
	{
		std::ifstream* file = new std::ifstream(path, std::ios::binary);
		if (!file->is_open())
		{
			delete file;
			return 0;
		}
		return reinterpret_cast<Rml::FileHandle>(file);
	}

	void Close(Rml::FileHandle file) override
	{
		auto* fs = reinterpret_cast<std::ifstream*>(file);
		if (fs)
		{
			fs->close();
			delete fs;
		}
	}

	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
	{
		auto* fs = reinterpret_cast<std::ifstream*>(file);
		if (!fs || !fs->is_open())
			return 0;
		fs->read(reinterpret_cast<char*>(buffer), size);
		return static_cast<size_t>(fs->gcount());
	}

	bool Seek(Rml::FileHandle file, long offset, int origin) override
	{
		auto* fs = reinterpret_cast<std::ifstream*>(file);
		if (!fs || !fs->is_open())
			return false;

		std::ios_base::seekdir dir;
		switch (origin)
		{
		case SEEK_SET: dir = std::ios::beg; break;
		case SEEK_CUR: dir = std::ios::cur; break;
		case SEEK_END: dir = std::ios::end; break;
		default: return false;
		}

		fs->seekg(offset, dir);
		return !fs->fail();
	}

	size_t Tell(Rml::FileHandle file) override
	{
		auto* fs = reinterpret_cast<std::ifstream*>(file);
		if (!fs || !fs->is_open())
			return 0;
		return static_cast<size_t>(fs->tellg());
	}
};

// =========================================================================
// RmlUIFileInterface
// =========================================================================
RmlUIFileInterface::RmlUIFileInterface()
{
	m_impl = new Impl();
}

RmlUIFileInterface::~RmlUIFileInterface()
{
	Uninstall();
	delete m_impl;
	m_impl = nullptr;
}

void RmlUIFileInterface::Install()
{
	if (m_impl)
	{
		Rml::SetFileInterface(m_impl);
	}
}

void RmlUIFileInterface::Uninstall()
{
	if (m_impl)
	{
		Rml::SetFileInterface(nullptr);
	}
}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
