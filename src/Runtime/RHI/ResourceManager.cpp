#include "ResourceManager.h"
#include "RenderRHI.h"
#include "Render/Core/RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)

ResourceManager* g_resource_manager = nullptr;

ResourceManager::ResourceManager() {}

void ResourceManager::ProcessPendingDestruction() {}

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
