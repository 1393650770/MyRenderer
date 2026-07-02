#include "VK_ResourcePool.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)

UInt64 VK_ResourcePool::HashTextureDesc(CONST TextureDesc& desc)
{
	UInt64 hash = 0;
	hash ^= static_cast<UInt64>(desc.format);
	hash ^= static_cast<UInt64>(desc.width)  << 16;
	hash ^= static_cast<UInt64>(desc.height) << 32;
	hash ^= static_cast<UInt64>(desc.type)   << 8;
	hash ^= static_cast<UInt64>(desc.samples) << 40;
	hash ^= static_cast<UInt64>(desc.mip_level) << 48;
	hash ^= static_cast<UInt64>(desc.usage)  << 24;
	hash ^= static_cast<UInt64>(desc.depth)  << 52;
	hash ^= static_cast<UInt64>(desc.layer_count) << 56;
	return hash;
}

Bool VK_ResourcePool::TextureDescMatch(CONST TextureDesc& a, CONST TextureDesc& b)
{
	return a.format   == b.format &&
	       a.width    == b.width  &&
	       a.height   == b.height &&
	       a.type     == b.type   &&
	       a.samples  == b.samples &&
	       a.mip_level == b.mip_level &&
	       a.usage    == b.usage  &&
	       a.depth    == b.depth  &&
	       a.layer_count == b.layer_count;
}

UInt64 VK_ResourcePool::HashBufferDesc(CONST BufferDesc& desc)
{
	UInt64 hash = 0;
	hash ^= static_cast<UInt64>(desc.size);
	hash ^= static_cast<UInt64>(desc.stride) << 16;
	hash ^= static_cast<UInt64>(static_cast<UInt32>(desc.type)) << 32;
	return hash;
}

Bool VK_ResourcePool::BufferDescMatch(CONST BufferDesc& a, CONST BufferDesc& b)
{
	return a.size   == b.size &&
	       a.stride == b.stride &&
	       a.type   == b.type;
}

std::unique_ptr<VK_Texture> VK_ResourcePool::AcquireTexture(CONST TextureDesc& desc)
{
	UInt64 key = HashTextureDesc(desc);
	auto it = texture_pool.find(key);
	if (it != texture_pool.end() && !it->second.empty())
	{
		auto tex = std::move(it->second.back());
		it->second.pop_back();
		return tex;
	}
	return nullptr;
}

void VK_ResourcePool::ReturnTexture(std::unique_ptr<VK_Texture> texture, CONST TextureDesc& desc)
{
	if (!texture)
		return;

	UInt64 key = HashTextureDesc(desc);
	auto& vec = texture_pool[key];
	if (vec.size() < MaxPooledPerKey)
	{
		vec.push_back(std::move(texture));
	}
	// else: pool full, texture will be destroyed when unique_ptr goes out of scope
}

std::unique_ptr<VK_Buffer> VK_ResourcePool::AcquireBuffer(CONST BufferDesc& desc)
{
	UInt64 key = HashBufferDesc(desc);
	auto it = buffer_pool.find(key);
	if (it != buffer_pool.end() && !it->second.empty())
	{
		auto buf = std::move(it->second.back());
		it->second.pop_back();
		return buf;
	}
	return nullptr;
}

void VK_ResourcePool::ReturnBuffer(std::unique_ptr<VK_Buffer> buffer, CONST BufferDesc& desc)
{
	if (!buffer)
		return;

	UInt64 key = HashBufferDesc(desc);
	auto& vec = buffer_pool[key];
	if (vec.size() < MaxPooledPerKey)
	{
		vec.push_back(std::move(buffer));
	}
	// else: pool full, buffer will be destroyed when unique_ptr goes out of scope
}



MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE

// -- [AI] Global VkDevice handle for RenderDoc debug object names
VkDevice g_debug_name_device = VK_NULL_HANDLE;

// ---------------------------------------------------------------------------
// Bridge functions – declared in the Render layer, implemented here.
// These use only RHI-level types so the Render layer has zero VK dependency.
// ---------------------------------------------------------------------------

#include "Render/Core/RenderGraphResource.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(Render)

// Global pool pointer – set by VK_Device::Init() (opaque void* to keep Render layer VK-free).
void* g_resource_pool = nullptr;

static inline MXRender::RHI::Vulkan::VK_ResourcePool* GetPool()
{
	return static_cast<MXRender::RHI::Vulkan::VK_ResourcePool*>(g_resource_pool);
}

std::unique_ptr<MXRender::RHI::Texture> AcquirePooledTexture(CONST MXRender::RHI::TextureDesc& desc)
{
	auto* pool = GetPool();
	if (!pool)
		return nullptr;

	auto vk_tex = pool->AcquireTexture(desc);
	if (vk_tex)
	{
		// VK_Texture derives from Texture; release into a Texture unique_ptr.
		return std::unique_ptr<MXRender::RHI::Texture>(vk_tex.release());
	}
	return nullptr;
}

void ReturnPooledTexture(std::unique_ptr<MXRender::RHI::Texture> texture, CONST MXRender::RHI::TextureDesc& desc)
{
	auto* pool = GetPool();
	if (!pool || !texture)
		return;

	// The Texture* is actually a VK_Texture*. Transfer ownership.
	auto* vk_raw = static_cast<MXRender::RHI::Vulkan::VK_Texture*>(texture.release());
	pool->ReturnTexture(std::unique_ptr<MXRender::RHI::Vulkan::VK_Texture>(vk_raw), desc);
}

std::unique_ptr<MXRender::RHI::Buffer> AcquirePooledBuffer(CONST MXRender::RHI::BufferDesc& desc)
{
	auto* pool = GetPool();
	if (!pool)
		return nullptr;

	auto vk_buf = pool->AcquireBuffer(desc);
	if (vk_buf)
	{
		return std::unique_ptr<MXRender::RHI::Buffer>(vk_buf.release());
	}
	return nullptr;
}

void ReturnPooledBuffer(std::unique_ptr<MXRender::RHI::Buffer> buffer, CONST MXRender::RHI::BufferDesc& desc)
{
	auto* pool = GetPool();
	if (!pool || !buffer)
		return;

	auto* vk_raw = static_cast<MXRender::RHI::Vulkan::VK_Buffer*>(buffer.release());
	pool->ReturnBuffer(std::unique_ptr<MXRender::RHI::Vulkan::VK_Buffer>(vk_raw), desc);
}

	// -- [AI:BEGIN] --
	// g_debug_name_device is in global scope, declared extern in RenderGraphResource.h
	void SetDebugNameForRHIResource(MXRender::RHI::RenderResource* resource, CONST String& name)
	{
		if (!resource || name.empty() || g_debug_name_device == VK_NULL_HANDLE) return;
		if (auto* tex = dynamic_cast<MXRender::RHI::Texture*>(resource))
		{
			auto* vk_tex = static_cast<MXRender::RHI::Vulkan::VK_Texture*>(tex);
			VkImage image = vk_tex->GetImage();
			if (image == VK_NULL_HANDLE) return;
			VkDebugUtilsObjectNameInfoEXT i{}; i.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			i.objectType = VK_OBJECT_TYPE_IMAGE; i.objectHandle = (UInt64)image; i.pObjectName = name.c_str();
			PFN_vkSetDebugUtilsObjectNameEXT fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(g_debug_name_device, "vkSetDebugUtilsObjectNameEXT");
			if (fn) fn(g_debug_name_device, &i);
		}
		else if (auto* buf = dynamic_cast<MXRender::RHI::Buffer*>(resource))
		{
			auto* vk_buf = static_cast<MXRender::RHI::Vulkan::VK_Buffer*>(buf);
			VkBuffer h = vk_buf->GetBuffer();
			if (h == VK_NULL_HANDLE) return;
			VkDebugUtilsObjectNameInfoEXT i{}; i.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			i.objectType = VK_OBJECT_TYPE_BUFFER; i.objectHandle = (UInt64)h; i.pObjectName = name.c_str();
			PFN_vkSetDebugUtilsObjectNameEXT fn = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(g_debug_name_device, "vkSetDebugUtilsObjectNameEXT");
			if (fn) fn(g_debug_name_device, &i);
		}
	}
	// -- [AI:END] --

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE
