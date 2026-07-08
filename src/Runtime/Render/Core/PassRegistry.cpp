#include "Render/Core/PassRegistry.h"
#include <algorithm>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

PassRegistry& PassRegistry::Get()
{
	static PassRegistry instance;
	return instance;
}

void PassRegistry::Register(const PassRegistryEntry& entry)
{
	m_entries.push_back(entry);
	m_by_category[entry.category].push_back(entry);
}

const PassRegistryEntry* PassRegistry::Find(CONST String& name) const
{
	for (auto& e : m_entries)
		if (e.name == name) return &e;
	return nullptr;
}

const Vector<PassRegistryEntry>& PassRegistry::GetByCategory(CONST String& category) const
{
	static Vector<PassRegistryEntry> empty;
	auto it = m_by_category.find(category);
	return (it != m_by_category.end()) ? it->second : empty;
}

Vector<String> PassRegistry::GetCategories() const
{
	Vector<String> cats;
	for (auto& kv : m_by_category)
		cats.push_back(kv.first);
	return cats;
}

// Built-in pass registrations
REGISTER_PASS(GBufferPass,     "Geometry",    RDGPassKind::Graphics)
REGISTER_PASS(DepthPrePass,    "Geometry",    RDGPassKind::Graphics)
REGISTER_PASS(ShadowPass,      "Geometry",    RDGPassKind::Graphics)
REGISTER_PASS(DirectLighting,  "Lighting",    RDGPassKind::Graphics)
REGISTER_PASS(IndirectLighting,"Lighting",    RDGPassKind::Graphics)
REGISTER_PASS(SSAOPass,        "Compute",     RDGPassKind::Compute)
REGISTER_PASS(BloomPass,       "PostProcess", RDGPassKind::Graphics)
REGISTER_PASS(TAAPass,         "PostProcess", RDGPassKind::Graphics)
REGISTER_PASS(ToneMappingPass, "PostProcess", RDGPassKind::Graphics)
REGISTER_PASS(DownsamplePass,  "Compute",     RDGPassKind::Compute)
REGISTER_PASS(GPUCulling,      "Compute",     RDGPassKind::Compute)
REGISTER_PASS(CopyPass,        "Utility",     RDGPassKind::Copy)
REGISTER_PASS(CustomPass,      "Utility",     RDGPassKind::Custom)

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
