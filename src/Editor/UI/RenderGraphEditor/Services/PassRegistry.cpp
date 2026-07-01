#include "UI/RenderGraphEditor/Services/PassRegistry.h"
#include <algorithm>

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(UI)

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

REGISTER_PASS(GBufferPass,     "Geometry",    Render::RDGPassKind::Graphics)
REGISTER_PASS(DepthPrePass,    "Geometry",    Render::RDGPassKind::Graphics)
REGISTER_PASS(ShadowPass,      "Geometry",    Render::RDGPassKind::Graphics)
REGISTER_PASS(DirectLighting,  "Lighting",    Render::RDGPassKind::Graphics)
REGISTER_PASS(IndirectLighting,"Lighting",    Render::RDGPassKind::Graphics)
REGISTER_PASS(SSAOPass,        "Compute",     Render::RDGPassKind::Compute)
REGISTER_PASS(BloomPass,       "PostProcess", Render::RDGPassKind::Graphics)
REGISTER_PASS(TAAPass,         "PostProcess", Render::RDGPassKind::Graphics)
REGISTER_PASS(ToneMappingPass, "PostProcess", Render::RDGPassKind::Graphics)
REGISTER_PASS(DownsamplePass,  "Compute",     Render::RDGPassKind::Compute)
REGISTER_PASS(GPUCulling,      "Compute",     Render::RDGPassKind::Compute)
REGISTER_PASS(CopyPass,        "Utility",     Render::RDGPassKind::Copy)
REGISTER_PASS(CustomPass,      "Utility",     Render::RDGPassKind::Custom)

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
