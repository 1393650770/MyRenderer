#pragma once
#ifndef _RENDERGRAPH_PASSREGISTRY_
#define _RENDERGRAPH_PASSREGISTRY_

#include "Core/ConstDefine.h"
#include "Render/Core/RenderGraphDefinition.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

struct PassRegistryEntry
{
	String name;
	String category;
	String description;
	RDGPassKind pass_kind = RDGPassKind::Graphics;
	RDGPassFlags pass_flags = RDGPassFlags::Raster;
};

// Global pass registry. Passes register themselves via REGISTER_PASS macro.
// Editor consults this for: categorized menus, Quick Create suggestions, template auto-fill.
MYRENDERER_BEGIN_CLASS(PassRegistry)
public:
	static PassRegistry& METHOD(Get)();

	void METHOD(Register)(const PassRegistryEntry& entry);
	const PassRegistryEntry* METHOD(Find)(CONST String& name) const;
	const Vector<PassRegistryEntry>& METHOD(GetAll)() const { return m_entries; }
	const Vector<PassRegistryEntry>& METHOD(GetByCategory)(CONST String& category) const;
	Vector<String> METHOD(GetCategories)() const;

private:
	PassRegistry() MYDEFAULT;
	Vector<PassRegistryEntry> m_entries;
	Map<String, Vector<PassRegistryEntry>> m_by_category;
MYRENDERER_END_CLASS

// Macro for static pass registration
#define REGISTER_PASS(Name, Category, Kind) \
	static struct RegPass_##Name { \
		RegPass_##Name() { \
			PassRegistryEntry e; \
			e.name = #Name; \
			e.category = Category; \
			e.pass_kind = Kind; \
			e.pass_flags = RDGPassFlags::Raster; \
			MXRender::Render::PassRegistry::Get().Register(e); \
		} \
	} g_RegPass_##Name;

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
#endif
