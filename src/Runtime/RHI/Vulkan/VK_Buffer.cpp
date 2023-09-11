#include"VK_Buffer.h"
#include "VK_Define.h"
#include "VK_Utils.h"

MYRENDERER_BEGIN_NAMESPACE(MXRender)
MYRENDERER_BEGIN_NAMESPACE(RHI)
MYRENDERER_BEGIN_NAMESPACE(Vulkan)
void* VK_Buffer::Map()
{
	return allocation.GetMappedPointer(device);
}

void VK_Buffer::Unmap()
{
}

VK_Buffer::VK_Buffer(VK_Device& in_device, const BufferDesc& in_buffer_desc) :Buffer(in_buffer_desc),
device(&in_device)
{
	VkBufferCreateInfo buffer_create_info = {};
	GenerateBufferCreateInfo(buffer_create_info, in_buffer_desc);
	vkCreateBuffer(device->GetDevice(), &buffer_create_info, VULKAN_CPU_ALLOCATOR, &buffer);

	device->GetMemoryManager()->AllocateBufferMemory(allocation, buffer, 
		TranslateBufferTypeToVulkanAllocationFlags(buffer_desc.type), 0);

}

void VK_Buffer::GenerateBufferCreateInfo(VkBufferCreateInfo& buffer_create_info, const BufferDesc& desc)
{
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = desc.size;
	buffer_create_info.usage =VK_Utils::Translate_Buffer_usage_type_To_VulkanUsageFlag(desc.type); //VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
}

void VK_Buffer::Destroy()
{

}



ENUM_VulkanAllocationFlags VK_Buffer::TranslateBufferTypeToVulkanAllocationFlags(const ENUM_BUFFER_TYPE& buffer_usage)
{
	ENUM_VulkanAllocationFlags allocation_flags = ENUM_VulkanAllocationFlags::None;
	switch (buffer_usage)
	{
	case ENUM_BUFFER_TYPE::Index:
	{
		allocation_flags= allocation_flags| ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	case ENUM_BUFFER_TYPE::Vertex:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::Uniform:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind ;
		break;
	}
	case ENUM_BUFFER_TYPE::Staging:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind | ENUM_VulkanAllocationFlags::HostVisible;
		break;
	}
	case ENUM_BUFFER_TYPE::None:
	{
		allocation_flags = allocation_flags | ENUM_VulkanAllocationFlags::AutoBind;
		break;
	}
	default:
		CHECK_WITH_LOG(true, "RHI Error :  Not support this buffer usage type !")
		break;
	}
	return allocation_flags;
}

VK_BufferManager::VK_BufferManager(VK_Device* in_device) :device(in_device)
{

}

MYRENDERER_END_NAMESPACE

MYRENDERER_END_NAMESPACE
MYRENDERER_END_NAMESPACE